#include <Arduino.h>
#include <driver/i2s.h>
#include "config.h"

// Hardware Serial 2 for Optical VLC Reception
HardwareSerial VlcSerial(2);

// Playback & System State
volatile bool isPlaying       = true;
volatile uint8_t currentVolume = 75; // 0 to 100%
volatile uint8_t currentTrack  = 0;
volatile uint8_t activeSpeaker = 0;  // 0 = Speaker A, 1 = Speaker B

// Track Synthesizer Frequencies (Pentatonic Demo Frequencies in Hz)
const float trackFrequencies[] = { 261.63f, 329.63f, 392.00f, 523.25f, 659.25f };
const int numTracks = sizeof(trackFrequencies) / sizeof(trackFrequencies[0]);

// FreeRTOS Task Handles
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t vlcTaskHandle   = NULL;

// Forward Declarations
void initI2S();
void initServo();
void setServoAngle(int angle);
void applySpeakerSelection(uint8_t spk);
void processVlcCommand(uint8_t opcode, uint8_t value);
void audioTask(void *param);
void vlcReceiverTask(void *param);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("=================================================="));
  Serial.println(F("  EW VLC AUDIO BASE STATION - DUAL I2S RECEIVER   "));
  Serial.println(F("=================================================="));

  // Configure Status & Control Pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_AMP1_SD, OUTPUT);
  pinMode(PIN_AMP2_SD, OUTPUT);

  // Default: Enable Speaker A (Left), Standby Speaker B (Right)
  applySpeakerSelection(0);

  // Initialize Actuator & Optical Bus
  initServo();
  setServoAngle(90); // Center position

  // Configure VLC Optical Receiver on GPIO 4 (RX only, TX unused)
  VlcSerial.begin(VLC_BAUD_RATE, SERIAL_8N1, PIN_VLC_RX, -1);

  // Initialize Hardware I2S Bus for MAX98357A Amplifiers
  initI2S();

  // Create Dedicated FreeRTOS Tasks across Dual Cores
  // Core 0: Protocol Receiver & Servo Management
  xTaskCreatePinnedToCore(
    vlcReceiverTask,
    "VlcTask",
    4096,
    NULL,
    2, // High priority for serial decoding
    &vlcTaskHandle,
    0  // Core 0
  );

  // Core 1: Real-Time Audio Pipeline (Zero-jitter playback)
  xTaskCreatePinnedToCore(
    audioTask,
    "AudioTask",
    8192,
    NULL,
    3, // Highest priority to prevent audio underrun
    &audioTaskHandle,
    1  // Core 1
  );

  Serial.println(F("[SYSTEM] Initialization Complete. Optical receiver listening."));
}

void loop() {
  // Empty loop: All work handled by FreeRTOS tasks on Core 0 & Core 1
  vTaskDelay(pdMS_TO_TICKS(1000));
}

// ==========================================
// I2S HARDWARE INITIALIZATION
// ==========================================
void initI2S() {
  i2s_config_t i2sConfig = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = AUDIO_SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 128,
    .use_apll             = false,
    .tx_desc_auto_clear   = true,
    .fixed_mclk           = 0
  };

  i2s_pin_config_t pinConfig = {
    .bck_io_num   = PIN_I2S_BCLK,
    .ws_io_num    = PIN_I2S_LRC,
    .data_out_num = PIN_I2S_DIN,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };

  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Failed to install I2S driver: 0x%04X\n", err);
    return;
  }

  err = i2s_set_pin(I2S_NUM_0, &pinConfig);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Failed to set I2S pins: 0x%04X\n", err);
    return;
  }

  i2s_start(I2S_NUM_0);
  Serial.println(F("[AUDIO] I2S Controller configured for dual MAX98357A."));
}

// ==========================================
// SERVO PWM INITIALIZATION & CONTROL
// ==========================================
void initServo() {
  ledcSetup(SERVO_LEDC_CHANNEL, SERVO_LEDC_FREQ, SERVO_LEDC_RES);
  ledcAttachPin(PIN_SERVO_PWM, SERVO_LEDC_CHANNEL);
  Serial.println(F("[SERVO] SG90 PWM initialized on GPIO 18."));
}

void setServoAngle(int angle) {
  angle = constrain(angle, 0, 180);
  // Map angle (0 - 180) to duty cycle
  uint32_t duty = map(angle, 0, 180, SERVO_MIN_DUTY, SERVO_MAX_DUTY);
  ledcWrite(SERVO_LEDC_CHANNEL, duty);
  Serial.printf("[SERVO] Position updated to: %d° (Duty: %u)\n", angle, duty);
}

// ==========================================
// SPEAKER ROUTING LOGIC
// ==========================================
void applySpeakerSelection(uint8_t spk) {
  activeSpeaker = spk;
  if (spk == 0) {
    // Speaker A Active (Left Channel), Speaker B Muted
    digitalWrite(PIN_AMP1_SD, HIGH);
    digitalWrite(PIN_AMP2_SD, LOW);
    Serial.println(F("[AUDIO ROUTING] Active: SPEAKER A (Left) | Standby: SPEAKER B"));
  } else {
    // Speaker B Active (Right Channel), Speaker A Muted
    digitalWrite(PIN_AMP1_SD, LOW);
    digitalWrite(PIN_AMP2_SD, HIGH);
    Serial.println(F("[AUDIO ROUTING] Active: SPEAKER B (Right) | Standby: SPEAKER A"));
  }
}

// ==========================================
// CORE 0 TASK: VLC OPTICAL RECEIVER
// ==========================================
void vlcReceiverTask(void *param) {
  uint8_t rxBuffer[6];
  int rxIndex = 0;

  while (1) {
    while (VlcSerial.available()) {
      uint8_t b = VlcSerial.read();

      // State 0: Wait for Sync Byte 1
      if (rxIndex == 0) {
        if (b == VLC_SYNC_BYTE_1) {
          rxBuffer[rxIndex++] = b;
        }
      }
      // State 1: Wait for Sync Byte 2
      else if (rxIndex == 1) {
        if (b == VLC_SYNC_BYTE_2) {
          rxBuffer[rxIndex++] = b;
        } else {
          rxIndex = 0; // Invalid sync sequence, reset
        }
      }
      // State 2: Buffer payload bytes (DEV_ID, OPCODE, VALUE, CHECKSUM)
      else if (rxIndex < 6) {
        rxBuffer[rxIndex++] = b;

        // Complete packet received
        if (rxIndex == 6) {
          uint8_t devId    = rxBuffer[2];
          uint8_t opcode   = rxBuffer[3];
          uint8_t value    = rxBuffer[4];
          uint8_t checksum = rxBuffer[5];

          // Validate Checksum: DEV_ID ^ OPCODE ^ VALUE
          uint8_t expectedChecksum = devId ^ opcode ^ value;
          if (checksum == expectedChecksum && devId == VLC_DEVICE_ID) {
            digitalWrite(PIN_STATUS_LED, HIGH);
            processVlcCommand(opcode, value);
            vTaskDelay(pdMS_TO_TICKS(15));
            digitalWrite(PIN_STATUS_LED, LOW);
          } else {
            Serial.printf("[VLC RX] Packet Checksum Mismatch! Got 0x%02X, expected 0x%02X\n",
                          checksum, expectedChecksum);
          }
          rxIndex = 0; // Reset parser
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// ==========================================
// COMMAND DISPATCHER
// ==========================================
void processVlcCommand(uint8_t opcode, uint8_t value) {
  Serial.printf("[CMD RECEIVED] Opcode: 0x%02X | Val: 0x%02X -> ", opcode, value);

  switch (opcode) {
    case CMD_PLAY_PAUSE:
      isPlaying = !isPlaying;
      Serial.printf("PLAY/PAUSE Toggled (%s)\n", isPlaying ? "PLAYING" : "PAUSED");
      break;

    case CMD_VOL_UP:
      currentVolume = min(100, currentVolume + 5);
      Serial.printf("Volume UP: %d%%\n", currentVolume);
      break;

    case CMD_VOL_DOWN:
      currentVolume = max(0, currentVolume - 5);
      Serial.printf("Volume DOWN: %d%%\n", currentVolume);
      break;

    case CMD_TRACK_NEXT:
      currentTrack = (currentTrack + 1) % numTracks;
      Serial.printf("Next Track: Index %d (Freq: %.1f Hz)\n", currentTrack, trackFrequencies[currentTrack]);
      break;

    case CMD_TRACK_PREV:
      currentTrack = (currentTrack == 0) ? (numTracks - 1) : (currentTrack - 1);
      Serial.printf("Previous Track: Index %d (Freq: %.1f Hz)\n", currentTrack, trackFrequencies[currentTrack]);
      break;

    case CMD_SELECT_SPK_A:
      applySpeakerSelection(0);
      break;

    case CMD_SELECT_SPK_B:
      applySpeakerSelection(1);
      break;

    case CMD_SERVO_AIM:
      setServoAngle((int)value);
      break;

    case CMD_MUTE_ALL:
      digitalWrite(PIN_AMP1_SD, LOW);
      digitalWrite(PIN_AMP2_SD, LOW);
      Serial.println(F("MUTE ALL: Both amplifiers shutdown."));
      break;

    default:
      Serial.printf("Unknown Opcode 0x%02X\n", opcode);
      break;
  }
}

// ==========================================
// CORE 1 TASK: REAL-TIME AUDIO SYNTHESIS & I2S STREAMING
// ==========================================
void audioTask(void *param) {
  const int bufferSamples = 128;
  int16_t audioBuffer[bufferSamples * 2]; // Stereo interleaved [L, R, L, R, ...]
  float phase = 0.0f;

  while (1) {
    if (!isPlaying || currentVolume == 0) {
      // Stream zeroed silence to maintain I2S clock stability
      memset(audioBuffer, 0, sizeof(audioBuffer));
    } else {
      float freq = trackFrequencies[currentTrack];
      float phaseIncrement = (2.0f * PI * freq) / AUDIO_SAMPLE_RATE;
      float amplitude = (currentVolume / 100.0f) * 16000.0f; // Scale to 16-bit signed

      for (int i = 0; i < bufferSamples; i++) {
        int16_t sample = (int16_t)(sin(phase) * amplitude);
        phase += phaseIncrement;
        if (phase >= 2.0f * PI) {
          phase -= 2.0f * PI;
        }

        // Stereo interleaved samples
        audioBuffer[i * 2]     = sample; // Left Channel
        audioBuffer[i * 2 + 1] = sample; // Right Channel
      }
    }

    size_t bytesWritten = 0;
    i2s_write(I2S_NUM_0, audioBuffer, sizeof(audioBuffer), &bytesWritten, portMAX_DELAY);
  }
}

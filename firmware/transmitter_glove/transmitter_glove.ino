#include <Wire.h>
#include "config.h"

// MPU-6050 Register Addresses
#define MPU6050_ADDR         0x68
#define MPU6050_SMPLRT_DIV   0x19
#define MPU6050_CONFIG       0x1A
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_PWR_MGMT_1   0x6B

// Hardware Serial 2 for Optical VLC Transmission over Light
HardwareSerial VlcSerial(2);

// Filter State Variables
float pitch = 0.0f;
float roll  = 0.0f;
unsigned long lastSampleTime = 0;

// Gesture State Machine Variables
uint8_t currentCandidate = CMD_NONE;
unsigned long candidateStartTime = 0;
unsigned long lastCommandSentTime = 0;

// Forward Declarations
void initMPU6050();
void readSensors(float &outPitch, float &outRoll, int &outFlexIdx, int &outFlexMid);
uint8_t classifyGesture(float p, float r, int flexIdx, int flexMid, uint8_t &outValue);
void transmitVlcPacket(uint8_t opcode, uint8_t value);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("=================================================="));
  Serial.println(F("  EW GLOVE TRANSMITTER - VISIBLE LIGHT COMM (VLC) "));
  Serial.println(F("=================================================="));

  // Configure Pins
  pinMode(PIN_VLC_LED, OUTPUT);
  digitalWrite(PIN_VLC_LED, LOW);
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  pinMode(PIN_FLEX_INDEX, INPUT);
  pinMode(PIN_FLEX_MIDDLE, INPUT);
  analogReadResolution(12); // ESP32 12-bit ADC (0 - 4095)

  // Configure Hardware Serial for VLC Optical Channel
  // TX on PIN_VLC_LED (GPIO 17), RX unused (-1)
  VlcSerial.begin(VLC_BAUD_RATE, SERIAL_8N1, -1, PIN_VLC_LED);

  // Initialize I2C Bus for MPU-6050
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000); // 400 kHz Fast I2C
  initMPU6050();

  lastSampleTime = millis();
  Serial.println(F("[SYSTEM] Initialization Complete. Optical transmitter active."));
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }
  float dt = (now - lastSampleTime) / 1000.0f;
  lastSampleTime = now;

  // 1. Read Sensors & Compute Orientation
  int flexIndexRaw = 0;
  int flexMiddleRaw = 0;
  readSensors(pitch, roll, flexIndexRaw, flexMiddleRaw);

  // 2. Classify Gesture Candidate
  uint8_t payloadValue = 0x00;
  uint8_t detected = classifyGesture(pitch, roll, flexIndexRaw, flexMiddleRaw, payloadValue);

  // 3. Debounce & State Machine
  if (detected != CMD_NONE) {
    if (detected == currentCandidate) {
      // Check if sustained for the required stability window
      if ((now - candidateStartTime >= GESTURE_STABLE_TIME_MS) &&
          (now - lastCommandSentTime >= GESTURE_COOLDOWN_MS)) {
        
        // Transmit over optical channel
        transmitVlcPacket(detected, payloadValue);
        lastCommandSentTime = now;
        currentCandidate = CMD_NONE; // Reset after trigger
      }
    } else {
      currentCandidate = detected;
      candidateStartTime = now;
    }
  } else {
    currentCandidate = CMD_NONE;
  }

  // Periodic Telemetry Output (Every 200 ms)
  static unsigned long lastLog = 0;
  if (now - lastLog > 200) {
    lastLog = now;
    Serial.printf("[TELEMETRY] Pitch: %+6.1f° | Roll: %+6.1f° | FlexIdx: %4d | FlexMid: %4d\n",
                  pitch, roll, flexIndexRaw, flexMiddleRaw);
  }
}

// ==========================================
// MPU-6050 HARDWARE INITIALIZATION
// ==========================================
void initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_PWR_MGMT_1);
  Wire.write(0x00); // Wake up device (clear SLEEP bit)
  uint8_t err = Wire.endTransmission();

  if (err != 0) {
    Serial.printf("[ERROR] MPU-6050 not found on I2C address 0x%02X! Check wiring.\n", MPU6050_ADDR);
    while (1) {
      digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
      delay(150);
    }
  }

  // Accelerometer Config: ±2g
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_CONFIG);
  Wire.write(0x00);
  Wire.endTransmission();

  // Gyroscope Config: ±250 °/s
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_GYRO_CONFIG);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println(F("[SENSOR] MPU-6050 successfully initialized."));
}

// ==========================================
// SENSOR READING & COMPLEMENTARY FILTER
// ==========================================
void readSensors(float &outPitch, float &outRoll, int &outFlexIdx, int &outFlexMid) {
  // Read Flex Sensors (Multisample average for noise immunity)
  long sumIdx = 0, sumMid = 0;
  for (int i = 0; i < 4; i++) {
    sumIdx += analogRead(PIN_FLEX_INDEX);
    sumMid += analogRead(PIN_FLEX_MIDDLE);
  }
  outFlexIdx = sumIdx / 4;
  outFlexMid = sumMid / 4;

  // Read MPU-6050 Accelerometer & Gyroscope
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)14, (uint8_t)true);

  if (Wire.available() >= 14) {
    int16_t axRaw = (Wire.read() << 8) | Wire.read();
    int16_t ayRaw = (Wire.read() << 8) | Wire.read();
    int16_t azRaw = (Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read(); // Skip temperature
    int16_t gxRaw = (Wire.read() << 8) | Wire.read();
    int16_t gyRaw = (Wire.read() << 8) | Wire.read();
    int16_t gzRaw = (Wire.read() << 8) | Wire.read();

    // Convert raw values to physical units (g and °/s)
    float ax = axRaw / 16384.0f;
    float ay = ayRaw / 16384.0f;
    float az = azRaw / 16384.0f;
    float gx = gxRaw / 131.0f;
    float gy = gyRaw / 131.0f;

    // Accelerometer Euler angles
    float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
    float rollAcc  = atan2(ay, az) * 180.0f / PI;

    // Complementary Filter
    float dt = SAMPLE_INTERVAL_MS / 1000.0f;
    outPitch = COMPLEMENTARY_ALPHA * (outPitch + gy * dt) + (1.0f - COMPLEMENTARY_ALPHA) * pitchAcc;
    outRoll  = COMPLEMENTARY_ALPHA * (outRoll  + gx * dt) + (1.0f - COMPLEMENTARY_ALPHA) * rollAcc;
  }
}

// ==========================================
// GESTURE CLASSIFIER
// ==========================================
uint8_t classifyGesture(float p, float r, int flexIdx, int flexMid, uint8_t &outValue) {
  outValue = 0x00;

  bool isIndexBent  = (flexIdx < FLEX_THRESHOLD_BENT);
  bool isMiddleBent = (flexMid < FLEX_THRESHOLD_BENT);
  bool isIndexStraight  = (flexIdx > FLEX_THRESHOLD_STRAIGHT);
  bool isMiddleStraight = (flexMid > FLEX_THRESHOLD_STRAIGHT);

  // Gesture 1: Fist Clench (Both fingers bent) -> Play / Pause
  if (isIndexBent && isMiddleBent) {
    return CMD_PLAY_PAUSE;
  }

  // Gesture 2: Index Point (Index straight, middle curled) -> Select Speaker A
  if (isIndexStraight && isMiddleBent) {
    return CMD_SELECT_SPK_A;
  }

  // Gesture 3: Peace / V-Sign (Both fingers extended) -> Select Speaker B
  if (isIndexStraight && isMiddleStraight) {
    // If hand is rotated significantly, aim servo
    if (r > 20.0f && r < 160.0f && abs(p) < 20.0f) {
      outValue = (uint8_t)constrain((int)r, 0, 180);
      return CMD_SERVO_AIM;
    }
  }

  // Gesture 4: Pitch Tilts (Fingers in neutral/open position)
  if (!isIndexBent && !isMiddleBent) {
    if (p > PITCH_THRESHOLD_UP) {
      outValue = 0x01; // Step increment
      return CMD_VOL_UP;
    }
    if (p < PITCH_THRESHOLD_DOWN) {
      outValue = 0x01; // Step decrement
      return CMD_VOL_DOWN;
    }
    if (r < ROLL_THRESHOLD_LEFT) {
      return CMD_TRACK_PREV;
    }
    if (r > ROLL_THRESHOLD_RIGHT) {
      return CMD_TRACK_NEXT;
    }
  }

  return CMD_NONE;
}

// ==========================================
// VLC OPTICAL PACKET TRANSMISSION
// ==========================================
void transmitVlcPacket(uint8_t opcode, uint8_t value) {
  // Compute XOR checksum: DEV_ID ^ OPCODE ^ VALUE
  uint8_t checksum = VLC_DEVICE_ID ^ opcode ^ value;

  uint8_t packet[6];
  packet[0] = VLC_SYNC_BYTE_1;
  packet[1] = VLC_SYNC_BYTE_2;
  packet[2] = VLC_DEVICE_ID;
  packet[3] = opcode;
  packet[4] = value;
  packet[5] = checksum;

  digitalWrite(PIN_STATUS_LED, HIGH);

  // Transmit 6-byte optical packet over Visible LED
  VlcSerial.write(packet, sizeof(packet));
  VlcSerial.flush(); // Ensure transmission complete

  digitalWrite(PIN_STATUS_LED, LOW);

  Serial.printf("[VLC TX] Sent Opcode: 0x%02X | Val: 0x%02X | Chk: 0x%02X\n",
                opcode, value, checksum);
}

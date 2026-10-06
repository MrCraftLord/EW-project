// =========================
// VLC TRANSMITTER (BOOSTED)
// Prints ONLY LED brightness
// =========================

// --- Pin Definitions ---
#define AUDIO_IN_PIN 34   // ADC pin for audio input
#define LED_OUT_PIN 22    // LED output pin (PWM)

// --- LED PWM Setup ---
#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION 8
#define LEDC_FREQ 50000   // 50 kHz VLC carrier

// --- Serial Debug Timer ---
unsigned long lastPrintTime = 0;
const long printInterval = 100;   // Print 10 times/sec (smooth but not slow)

// =========================
// DSP PARAMETERS (TUNABLE)
// =========================
float softwareGain = 3.0f;        // Increase if variation is small
float maxGain = 12.0f;
float minGain = 0.5f;
float targetPeak = 300.0f;

float peakAttack = 0.02f;
float peakRelease = 0.001f;

float hpAlpha = 0.995f;

// RENAMED to avoid conflict with gamma() in math.h
float gammaExp = 1.6f;            // Dynamic expansion (makes quiet parts louder)

int pwmMax = 255;

// =========================
// DSP INTERNAL STATE
// =========================
float hp_prev = 0.0f;
float hp_y = 0.0f;
float peak = 1.0f;
float currentGain = 1.0f;

void setup() {
  Serial.begin(115200);

  pinMode(AUDIO_IN_PIN, INPUT);

  // Set up PWM for LED
  ledcSetup(LEDC_CHANNEL, LEDC_FREQ, LEDC_RESOLUTION);
  ledcAttachPin(LED_OUT_PIN, LEDC_CHANNEL);

  Serial.println("BOOSTED VLC Transmitter Ready.");
}

void loop() {
  // =========================
  // 1) Read audio signal
  // =========================
  int raw = analogRead(AUDIO_IN_PIN);   // 0..4095
  float x = float(raw);

  // =========================
  // 2) High-pass filter (DC removal)
  // =========================
  hp_y = hpAlpha * (hp_y + x - hp_prev);
  hp_prev = x;
  float filtered = hp_y;

  // =========================
  // 3) Software pre-gain
  // =========================
  float amplified = filtered * softwareGain;

  // =========================
  // 4) Envelope magnitude
  // =========================
  float env = fabs(amplified);

  // =========================
  // 5) Peak tracking (AGC envelope)
  // =========================
  if (env > peak) {
    peak += (env - peak) * peakAttack;   // attack
  } else {
    peak += (env - peak) * peakRelease;  // release
  }
  if (peak < 1.0f) peak = 1.0f;

  // =========================
  // 6) Automatic Gain Control (AGC)
  // =========================
  float desiredGain = targetPeak / peak;
  if (desiredGain > maxGain) desiredGain = maxGain;
  if (desiredGain < minGain) desiredGain = minGain;

  currentGain = currentGain * 0.95f + desiredGain * 0.05f;

  float processed = amplified * currentGain;

  // =========================
  // 7) Dynamic expansion (gamma curve)
  // =========================
  float normalized = processed / 2048.0f;   // → -1..1 range
  float env_norm = fabs(normalized);        // 0..1
  float env_gamma = pow(env_norm, 1.0f / gammaExp);
  float signed_expanded = (normalized >= 0) ? env_gamma : -env_gamma;

  // =========================
  // 8) Convert to PWM 0..255
  // =========================
  int pwm = int((signed_expanded * 0.5f + 0.5f) * pwmMax);
  pwm = constrain(pwm, 0, pwmMax);

  // =========================
  // 9) Output to LED
  // =========================
  ledcWrite(LEDC_CHANNEL, pwm);

  // =========================
  // 10) Print ONLY LED brightness
  // =========================
  unsigned long now = millis();
  if (now - lastPrintTime >= printInterval) {
    lastPrintTime = now;

    Serial.print("LED Brightness: ");
    Serial.println(pwm);     // JUST PWM VALUE
  }
}
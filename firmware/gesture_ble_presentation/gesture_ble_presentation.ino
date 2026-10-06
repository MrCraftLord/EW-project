#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <BleKeyboard.h>

// --- Bluetooth Device ---
BleKeyboard bleKeyboard("Gesture Player", "ESP32", 100);

// --- Pin Definitions ---
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 15
#define BUTTON_PIN 32
#define LED_PIN 2  // Onboard LED

// --- Gesture Thresholds ---
#define JOLT_RIGHT_THRESH 7.0
#define JOLT_LEFT_THRESH  -7.0
#define JOLT_DELTA_THRESH 6.0   // require a spike (delta) for slide detection
#define TILT_RIGHT_THRESH 6.0
#define TILT_LEFT_THRESH  -6.0
#define TILT_FRONT_THRESH 4.0
#define TILT_BACK_THRESH  -4.0
#define UPSIDE_DOWN_THRESH -8.0

// --- Timing Constants ---
const unsigned long debounceDelay        = 50;
const unsigned long actionCooldown       = 500;
const unsigned long slideDominanceTime   = 500;
const unsigned long smoothVolumeInterval = 200;
const unsigned long tiltHoldTime         = 350;
const unsigned long volumeLockoutAfterSlide = 700;  // delay before volume can change after a slide

Adafruit_MPU6050 mpu;

// --- State ---
bool gesturesEnabled = false;
bool buttonState = LOW, lastButtonReading = LOW;
String lastGesture = "";
unsigned long lastDebounceTime = 0;
unsigned long lastActionTime = 0;
unsigned long lastSlideTime = 0;
unsigned long lastVolumeAction = 0;

// --- Tilt-hold bookkeeping ---
unsigned long tiltStartTime = 0;
int tiltDirection = 0; // 1 = right, -1 = left, 0 = neutral

// --- Gyro Offsets ---
float gyroXoffset = 0, gyroYoffset = 0, gyroZoffset = 0;

// --- Previous accel for delta detection (hybrid) ---
float prevAx = 0.0;

// ---------------------------- Helper Functions ----------------------------

void calibrateGyro() {
  Serial.println("Calibrating Gyro... Keep still!");
  gyroXoffset = gyroYoffset = gyroZoffset = 0;
  for (int i = 0; i < 1000; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    gyroXoffset += g.gyro.x;
    gyroYoffset += g.gyro.y;
    gyroZoffset += g.gyro.z;
    delay(1);
  }
  gyroXoffset /= 1000;
  gyroYoffset /= 1000;
  gyroZoffset /= 1000;
  Serial.println("Calibration complete!");
}

void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        gesturesEnabled = !gesturesEnabled;
        Serial.println(gesturesEnabled ? "--- GESTURES ENABLED ---" : "--- GESTURES DISABLED ---");
        digitalWrite(LED_PIN, gesturesEnabled ? HIGH : LOW);
        lastGesture = "";
      }
    }
  }
  lastButtonReading = reading;
}

void performGestureAction(String gesture) {
  if (!bleKeyboard.isConnected()) return;
  if (millis() - lastActionTime < actionCooldown) return;

  if (gesture == "Slide Right") {
    Serial.println("▶️ Next Track");
    bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
    lastSlideTime = millis();
  }
  else if (gesture == "Slide Left") {
    Serial.println("⏮️ Previous Track");
    bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
    lastSlideTime = millis();
  }
  else if (gesture == "Tilt Front") {
    Serial.println("⏯️ Play / Pause");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  }
  else if (gesture == "Upside Down") {
    Serial.println("🔇 Mute");
    bleKeyboard.write(KEY_MEDIA_MUTE);
  }

  lastActionTime = millis();
}

// ---------------------------- Volume Handler ----------------------------

void handleSmoothVolume(float ax) {
  if (!bleKeyboard.isConnected() || !gesturesEnabled) return;

  unsigned long now = millis();

  // --- Lockout after slide ---
  if (now - lastSlideTime < volumeLockoutAfterSlide) {
    // Ignore volume changes briefly after slide
    tiltDirection = 0;
    tiltStartTime = 0;
    return;
  }

  // --- Detect start of tilt direction ---
  if (ax > TILT_RIGHT_THRESH) {
    if (tiltDirection != 1) { tiltDirection = 1; tiltStartTime = now; }
  }
  else if (ax < TILT_LEFT_THRESH) {
    if (tiltDirection != -1) { tiltDirection = -1; tiltStartTime = now; }
  }
  else {
    // back to neutral
    tiltDirection = 0;
    tiltStartTime = 0;
    return;
  }

  // --- Only act if tilt held long enough ---
  if (tiltDirection != 0 && (now - tiltStartTime) >= tiltHoldTime) {
    if (now - lastVolumeAction >= smoothVolumeInterval) {
      if (tiltDirection == 1) {
        Serial.println("🔊 Volume Up (Held)");
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      } else {
        Serial.println("🔉 Volume Down (Held)");
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      }
      lastVolumeAction = now;
    }
  }
}

// ---------------------------- SETUP ----------------------------

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  calibrateGyro();

  // initialize prevAx with a first reading to minimize startup spike detection
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  prevAx = a.acceleration.x;

  Serial.println("Starting BLE...");
  bleKeyboard.begin();
  Serial.println("System ready. Press button to enable gestures.");
}

// ---------------------------- LOOP ----------------------------

void loop() {
  checkButton();

  // LED indicator
  if (bleKeyboard.isConnected())
    digitalWrite(LED_PIN, gesturesEnabled ? HIGH : LOW);
  else
    digitalWrite(LED_PIN, millis() % 1000 < 200 ? HIGH : LOW);

  if (!gesturesEnabled) { delay(20); return; }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // raw accel and corrected gyro (we keep gyro offsets for future use)
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  // compute delta for hybrid jolt detection
  float deltaAx = ax - prevAx;
  prevAx = ax; // update for next loop

  String currentGesture = "";

  // --- Upside Down detection ---
  if (az < UPSIDE_DOWN_THRESH) {
    currentGesture = "Upside Down";
  }

  // --- Hybrid Slides (require flatness + sudden spike) ---
  else if (az > 8.0 && fabs(deltaAx) > JOLT_DELTA_THRESH) {
    if (deltaAx > 0 && ax > JOLT_RIGHT_THRESH) {
      currentGesture = "Slide Right";
      lastSlideTime = millis();
    } else if (deltaAx < 0 && ax < JOLT_LEFT_THRESH) {
      currentGesture = "Slide Left";
      lastSlideTime = millis();
    }
  }

  // --- Tilts (ignored briefly after slides) ---
  if (currentGesture == "") {
    if (millis() - lastSlideTime > slideDominanceTime) {
      if (ax > TILT_RIGHT_THRESH) currentGesture = "Tilt Right";
      else if (ax < TILT_LEFT_THRESH) currentGesture = "Tilt Left";
      else if (ay > TILT_FRONT_THRESH) currentGesture = "Tilt Front";
      else if (ay < TILT_BACK_THRESH) currentGesture = "Tilt Back";
      else currentGesture = "Neutral";
    } else {
      currentGesture = "Neutral";
    }
  }

  // Smooth volume runs in parallel (tilt-hold + post-slide lockout handled inside)
  handleSmoothVolume(ax);

  // --- Perform and print gesture ---
  if (currentGesture != lastGesture) {
    Serial.println(currentGesture);
    performGestureAction(currentGesture);
    lastGesture = currentGesture;
  }

  delay(50);
}
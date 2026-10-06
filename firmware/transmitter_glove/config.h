#ifndef CONFIG_TRANSMITTER_H
#define CONFIG_TRANSMITTER_H

#include <Arduino.h>

// ==========================================
// PIN ALLOCATIONS
// ==========================================
#define PIN_I2C_SDA         21    // MPU-6050 SDA
#define PIN_I2C_SCL         22    // MPU-6050 SCL

#define PIN_FLEX_INDEX      34    // ADC1_CH6 (Index finger flex sensor divider)
#define PIN_FLEX_MIDDLE     35    // ADC1_CH7 (Middle finger flex sensor divider)

#define PIN_VLC_LED         17    // Visible Light Emitter Driver (Base of 2N2222)
#define PIN_STATUS_LED      2     // Onboard debug LED

// ==========================================
// VLC PROTOCOL DEFINITIONS
// ==========================================
#define VLC_SYNC_BYTE_1     0xAA
#define VLC_SYNC_BYTE_2     0x55
#define VLC_DEVICE_ID       0x01

#define CMD_NONE            0x00
#define CMD_PLAY_PAUSE      0x10
#define CMD_VOL_UP          0x20
#define CMD_VOL_DOWN        0x21
#define CMD_TRACK_NEXT      0x30
#define CMD_TRACK_PREV      0x31
#define CMD_SELECT_SPK_A    0x40
#define CMD_SELECT_SPK_B    0x41
#define CMD_SERVO_AIM       0x50
#define CMD_MUTE_ALL        0x60

// Optical transmission baud rate (Baseband UART-over-Light)
#define VLC_BAUD_RATE       2400

// ==========================================
// KINEMATIC THRESHOLDS & CALIBRATION
// ==========================================
// Flex Sensor ADC Thresholds (12-bit ADC: 0 - 4095)
// Flat / Straight finger: ~2400 - 2800
// Bent 90 degrees: ~1400 - 1800
#define FLEX_THRESHOLD_BENT       1850
#define FLEX_THRESHOLD_STRAIGHT   2300

// IMU Tilt Angles (Degrees)
#define PITCH_THRESHOLD_UP        28.0f
#define PITCH_THRESHOLD_DOWN     -28.0f
#define ROLL_THRESHOLD_RIGHT      32.0f
#define ROLL_THRESHOLD_LEFT      -32.0f

// Filter constant for complementary filter
#define COMPLEMENTARY_ALPHA       0.96f

// Timing & Debouncing (Milliseconds)
#define SAMPLE_INTERVAL_MS        20    // 50 Hz sensor loop
#define GESTURE_STABLE_TIME_MS    60    // 3 consecutive identical samples
#define GESTURE_COOLDOWN_MS       350   // Refractory lockout period between commands

#endif // CONFIG_TRANSMITTER_H

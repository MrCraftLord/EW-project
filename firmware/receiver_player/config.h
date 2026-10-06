#ifndef CONFIG_RECEIVER_H
#define CONFIG_RECEIVER_H

#include <Arduino.h>

// ==========================================
// PIN ALLOCATIONS
// ==========================================
#define PIN_VLC_RX          4     // Optical Detector Digital Out (UART RX)
#define PIN_STATUS_LED      2     // Onboard Debug LED

// SG90 Micro Servo Actuator
#define PIN_SERVO_PWM       18    // LEDC PWM channel for servo

// I2S Audio Bus (Dual MAX98357A Amplifiers)
#define PIN_I2S_BCLK        22    // Bit Clock (Shared by Amp 1 & 2)
#define PIN_I2S_LRC         21    // Word Select / Left-Right Clock (Shared)
#define PIN_I2S_DIN         19    // Serial Data Out (Shared)

// Independent Amplifier Enable / Shutdown Controls
#define PIN_AMP1_SD         25    // Speaker A (Left) Enable: HIGH = On, LOW = Shutdown
#define PIN_AMP2_SD         26    // Speaker B (Right) Enable: HIGH = On, LOW = Shutdown

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

#define VLC_BAUD_RATE       2400

// ==========================================
// AUDIO & SERVO CONFIGURATION
// ==========================================
#define AUDIO_SAMPLE_RATE   44100
#define AUDIO_CHANNELS      2     // Stereo I2S
#define AUDIO_BITS_SAMPLE   16

// Servo LEDC PWM Settings
#define SERVO_LEDC_CHANNEL  0
#define SERVO_LEDC_FREQ     50    // 50 Hz standard servo update rate
#define SERVO_LEDC_RES      16    // 16-bit timer resolution (0 - 65535)

// Pulse width for 50Hz (20ms period, 65535 ticks):
// 0.5ms (0 deg)  -> (0.5 / 20.0) * 65535 = ~1638
// 2.5ms (180 deg) -> (2.5 / 20.0) * 65535 = ~8192
#define SERVO_MIN_DUTY      1638
#define SERVO_MAX_DUTY      8192

#endif // CONFIG_RECEIVER_H

# Bill of Materials (BOM) & Hardware Specifications

This document outlines the complete bill of materials, component specifications, pin allocations, and budget estimates for the **Gesture-Controlled Music Player using Visible Light Communication (VLC)**.

---

## 1. Master Component Summary Table

| Category | Component | Qty | Key Specifications | Estimated Cost (INR) | Estimated Cost (USD) |
|---|---|:---:|---|:---:|:---:|
| **Processing** | ESP32-WROOM-32 Dev Kit (38-pin or 30-pin) | 2 | Dual Tensilica Xtensa LX6 @ 240 MHz, 520 KB SRAM, 4 MB Flash, I²S & I²C hardware peripherals | ₹650 × 2 = ₹1,300 | ~$15.50 |
| **Motion Sensing** | MPU-6050 6-DOF IMU Breakout | 1 | 3-axis Accelerometer (±2g to ±16g), 3-axis Gyroscope (±250 to ±2000 °/s), Digital Motion Processor (DMP), I²C interface | ₹180 | ~$2.20 |
| **Flex Sensing** | 2.2" Flexible Bend Sensor | 2 | Variable resistance (Flat: ~10 kΩ, Bent 90°: ~30–40 kΩ) | ₹450 × 2 = ₹900 | ~$10.80 |
| **Audio Amplification** | MAX98357A I²S Class-D DAC Amplifier | 2 | 3.2W into 4Ω @ 5V, 1.4W into 8Ω @ 3.3V, 92% efficiency, 77 dB PSRR, 35 µV output noise | ₹190 × 2 = ₹380 | ~$4.60 |
| **Audio Output** | 3W 4Ω Full-Range Micro Speakers | 2 | 40mm diameter cone, rubber surround, 80 Hz – 20 kHz response | ₹120 × 2 = ₹240 | ~$2.90 |
| **Optical Emitter** | High-Brightness 5mm Visible LED (Blue 465nm / Red 650nm) | 2 | Luminous intensity: >8,000 mcd, Forward Voltage: 2.0V–3.2V, Forward Current: 20–30 mA | ₹10 × 2 = ₹20 | ~$0.25 |
| **Optical Driver** | 2N2222A NPN Transistor (or 2N7000 N-MOSFET) | 1 | $V_{CEO} = 40\text{V}$, $I_C = 800\text{mA}$, $h_{FE} = 100\text{--}300$ for fast optical switching | ₹10 | ~$0.12 |
| **Optical Receiver** | High-Speed Visible Photodiode Module with LM393 Comparator | 1 | Peak sensitivity: 400–700 nm, onboard multi-turn potentiometer threshold adjustment, Digital (DO) & Analog (AO) outputs | ₹90 | ~$1.10 |
| **Mechanical Servo** | SG90 9g Micro Servo Motor | 1 | Operating speed: 0.12 sec/60°, Stall torque: 1.8 kg·cm, 0°–180° rotation, PWM controlled | ₹130 | ~$1.60 |
| **Portable Power** | 3.7V 1000 mAh LiPo Battery (Model 103040 / 803040) | 1 | Lithium-polymer pouch cell, 3.7V nominal, 4.2V charging cutoff, integrated PCM protection | ₹350 | ~$4.20 |
| **Charge Controller** | TP4056 USB-C LiPo Charging Board | 1 | Constant-current/constant-voltage charger with DW01A battery protection and FS8205A dual MOSFET | ₹40 | ~$0.50 |
| **Wearable Substrate** | Half-Finger or Full Cotton/Spandex Glove | 1 | Breathable fabric with elastic backing for secure sensor mounting | ₹80 | ~$1.00 |
| **Passives** | Resistor Assortment Kit (22kΩ, 1kΩ, 220Ω, 100Ω) | 1 | 1/4W Metal Film Resistors (±1% tolerance) | ₹60 | ~$0.75 |
| **Prototyping** | Full-Size Solderless Breadboards + Jumper Wire Set | 2 | 830 tie-point breadboards, 120-piece M-M, M-F, F-F DuPont jumpers | ₹300 | ~$3.60 |
| **Power Supply** | 5V 2A USB Power Adapter + Micro-USB / USB-C Cables | 2 | Clean 5V DC power for stationary Central Unit & Transmitter charging | ₹250 | ~$3.00 |
| **Total Estimated Cost** | — | — | **Complete System Bill of Materials** | **₹4,250 INR** | **~$52.00 USD** |

---

## 2. Component Pinout & Operating Specifications

### 2.1. ESP32-WROOM-32 Pin Capabilities
* **Logic Level**: 3.3V LVTTL (Inputs are NOT 5V tolerant).
* **ADC**: 12-bit SAR ADC (Channels ADC1_CH0 through ADC1_CH7 on GPIO 32–39 are safe during Wi-Fi/Bluetooth operation).
* **I²C**: Any GPIO via matrix; standard default pins `GPIO 21` (SDA) and `GPIO 22` (SCL).
* **I²S Hardware Controller**: Supports PCM, I²S standard, and left/right justified audio streaming.

### 2.2. MPU-6050 6-Axis MotionTracking Device
* **Operating Voltage**: 3.3V – 5.0V (onboard 3.3V LDO on breakout).
* **Default I²C Address**: `0x68` (when `AD0` pin is connected to GND); `0x69` (when `AD0` is tied to VCC).
* **Communication Speed**: Fast-mode I²C up to 400 kHz.

### 2.3. MAX98357A I²S Mono Class-D Audio Amplifier
* **Input Format**: 16-bit, 24-bit, or 32-bit I²S digital audio data.
* **Sampling Rate ($F_s$)**: 8 kHz to 96 kHz.
* **Pins**:
  * `BCLK`: Bit Clock (determines sample bit rate).
  * `LRC` / `WS`: Word Select / Left-Right Clock (transitions indicate channel).
  * `DIN`: Serial Data Input.
  * `GAIN`: Gain selection (15dB, 12dB, 9dB, 6dB, 3dB via resistor pull to GND/VCC).
  * `SD_MODE`: Multi-function shutdown and channel selector:
    * Pulling to GND: Complete hardware shutdown ($I_Q < 2\ \mu\text{A}$).
    * Floating or pull-down: Left channel ($(\text{L}+\text{R})/2$).
    * With pull-up to VDD: Right channel.

---

## 3. Recommended Indian & International Sourcing Channels
* **India (Fast Regional Shipping)**:
  * [Robu.in](https://robu.in) (ESP32, MAX98357A, MPU-6050, Flex Sensors, TP4056)
  * [ElectronicsComp.com](https://www.electronicscomp.com) (Passives, breadboards, jumper wires)
  * [ThinkRobotics.in](https://thinkrobotics.in) (Sensors & audio modules)
* **International**:
  * [AliExpress](https://www.aliexpress.com) (Budget bulk components)
  * [Adafruit Industries](https://www.adafruit.com) (Original reference breakout modules)
  * [SparkFun Electronics](https://www.sparkfun.com) (Flex sensors & documentation)

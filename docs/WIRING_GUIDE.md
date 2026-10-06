# Hardware Wiring Guide & Pinout Diagrams

This guide provides the complete circuit schematics, wiring tables, resistor calculations, and power distribution diagrams for both the **Transmitter Glove** and the **Receiver Base Station**.

---

## 1. Transmitter Module (Smart Glove) Wiring

```
                         +-----------------------+
                         |      ESP32-WROOM      |
                         |     (Transmitter)     |
                         |                       |
   [3.3V] -------------- | 3.3V                  |
   [GND]  -------------- | GND                   |
                         |                       |
   [MPU-6050 SDA] ------ | GPIO 21 (I2C SDA)     |
   [MPU-6050 SCL] ------ | GPIO 22 (I2C SCL)     |
                         |                       |
   [Flex Sensor 1] ----- | GPIO 34 (ADC1_CH6)    |
   [Flex Sensor 2] ----- | GPIO 35 (ADC1_CH7)    |
                         |                       |
   [VLC LED Driver] ---- | GPIO 17 (TX Pulse)    |
                         +-----------------------+
```

### 1.1. Pin Connections Table (Transmitter Glove)

| Peripheral | Peripheral Pin | ESP32 Pin | Logic Level | Connection Details & Notes |
|---|---|---|:---:|---|
| **MPU-6050 IMU** | `VCC` | `3.3V` | 3.3V | Powered from ESP32 3.3V rail |
| | `GND` | `GND` | 0V | Common system ground |
| | `SCL` | `GPIO 22` | 3.3V | I²C Serial Clock |
| | `SDA` | `GPIO 21` | 3.3V | I²C Serial Data |
| | `AD0` | `GND` | 0V | Sets I²C slave address to `0x68` |
| | `INT` | *N/C* | — | Not connected (polling mode) |
| **Flex Sensor 1 (Index)** | Terminal 1 | `3.3V` | 3.3V | Voltage divider high side |
| | Terminal 2 | `GPIO 34` | Analog (0–3.3V) | Tap between sensor and 22 kΩ resistor |
| | Pull-down | `GND` | 0V | 22 kΩ 1/4W resistor to Ground |
| **Flex Sensor 2 (Middle)** | Terminal 1 | `3.3V` | 3.3V | Voltage divider high side |
| | Terminal 2 | `GPIO 35` | Analog (0–3.3V) | Tap between sensor and 22 kΩ resistor |
| | Pull-down | `GND` | 0V | 22 kΩ 1/4W resistor to Ground |
| **VLC LED Emitter** | Base Resistor | `GPIO 17` | 3.3V | 1 kΩ resistor to 2N2222 Base |
| | Transistor Emitter | `GND` | 0V | 2N2222 Emitter to Ground |
| | Transistor Collector| LED Cathode | — | 2N2222 Collector to LED (−) |
| | LED Anode | `3.3V` or `5V` | — | Through 100 Ω current-limiting resistor |

### 1.2. Flex Sensor Voltage Divider Calculation
The flex sensor behaves as a variable resistor ($R_{\text{flex}} \approx 10\text{ k}\Omega$ flat, $35\text{ k}\Omega$ bent 90°).
With a fixed pull-down resistor $R_1 = 22\text{ k}\Omega$:
$$V_{\text{out}} = V_{\text{CC}} \times \frac{R_1}{R_{\text{flex}} + R_1}$$
* **Flat ($R_{\text{flex}} \approx 10\text{ k}\Omega$)**:
  $$V_{\text{out}} = 3.3\text{V} \times \frac{22}{10 + 22} = 3.3\text{V} \times 0.6875 \approx 2.27\text{V} \quad (\text{ADC} \approx 2818)$$
* **Bent 90° ($R_{\text{flex}} \approx 35\text{ k}\Omega$)**:
  $$V_{\text{out}} = 3.3\text{V} \times \frac{22}{35 + 22} = 3.3\text{V} \times 0.3859 \approx 1.27\text{V} \quad (\text{ADC} \approx 1576)$$
This yields a clear dynamic range of $>1200$ ADC counts, enabling noise-immune threshold detection.

---

## 2. Receiver & Audio Station (Base Unit) Wiring

```
                         +-----------------------+
                         |      ESP32-WROOM      |
                         |      (Receiver)       |
                         |                       |
   [VLC Detector DO] --- | GPIO 4 (Pulse RX)     |
                         |                       |
   [SG90 Servo PWM] ---- | GPIO 18 (LEDC PWM)    |
                         |                       |
   [Amp 1 & 2 BCLK] ---- | GPIO 22 (I2S BCLK)    |
   [Amp 1 & 2 LRC] ----- | GPIO 21 (I2S LRC/WS)  |
   [Amp 1 & 2 DIN] ----- | GPIO 19 (I2S DIN)     |
                         |                       |
   [Amp 1 SD_MODE] ----- | GPIO 25 (Enable Ch A) |
   [Amp 2 SD_MODE] ----- | GPIO 26 (Enable Ch B) |
                         +-----------------------+
```

### 2.1. Pin Connections Table (Central Base Unit)

| Subsystem | Module Pin | ESP32 Pin | External Connection | Function & Notes |
|---|---|---|---|---|
| **VLC Optical Receiver** | `VCC` | `3.3V` or `5V` | System VCC | Photodiode module power |
| | `GND` | `GND` | System GND | Ground return |
| | `DO` | `GPIO 4` | — | Digital pulse output (comparator out) |
| **SG90 Micro Servo** | Signal (Yellow/Orange)| `GPIO 18` | — | 50 Hz PWM position control |
| | `VCC` (Red) | — | **5V External USB Rail** | Avoids ESP32 brownout during motor stall |
| | `GND` (Brown) | `GND` | System GND | Common ground return |
| **I²S Bus (Shared)** | `BCLK` (Both Amps) | `GPIO 22` | — | Audio bit clock (serial clock) |
| | `LRC` (Both Amps) | `GPIO 21` | — | Word select (left/right frame clock) |
| | `DIN` (Both Amps) | `GPIO 19` | — | Audio serial data bus |
| **MAX98357A Amp #1** | `VIN` | — | **5V External USB Rail** | Power supply for 3.2W output into 4Ω |
| | `GND` | `GND` | System GND | Common ground return |
| | `GAIN` | `GND` | — | Sets gain to +9 dB (or leave floating for +12 dB) |
| | `SD_MODE` | `GPIO 25` | — | High = Active, Low = Low-Power Shutdown |
| | `OUT+ / OUT-` | Speaker 1 | 3W 4Ω Speaker A | Differential audio drive (bridged tied load) |
| **MAX98357A Amp #2** | `VIN` | — | **5V External USB Rail** | Power supply for 3.2W output into 4Ω |
| | `GND` | `GND` | System GND | Common ground return |
| | `GAIN` | `GND` | — | Sets gain to +9 dB |
| | `SD_MODE` | `GPIO 26` | — | High = Active, Low = Low-Power Shutdown |
| | `OUT+ / OUT-` | Speaker 2 | 3W 4Ω Speaker B | Differential audio drive |

---

## 3. Power Distribution & Brownout Mitigation

```
                  +-----------------------------------+
                  |      5V 2A USB Power Source       |
                  +-----------------------------------+
                     |                             |
                     |                             +--------------------+
                     |                                                  |
           [100 µF Electrolytic]                              [100 µF Electrolytic]
           Filter Capacitor                                   Filter Capacitor
                     |                                                  |
           +---------+---------+                              +---------+---------+
           |                   |                              |                   |
      ESP32 VIN          SG90 Servo                     Amp 1 VIN           Amp 2 VIN
     (3.3V LDO)           (5V Motor)                    (Audio 5V)          (Audio 5V)
```

> [!IMPORTANT]
> **Common Ground Requirement**: All grounds (ESP32 GND, Servo GND, Amplifier GNDs, and Optical Receiver GND) **must be tied together** at a single star-ground point.
>
> **Decoupling Capacitors**: Servo motors induce inductive voltage spikes on the 5V line when starting and stopping. Place a **100 µF – 470 µF electrolytic capacitor** in parallel with a **0.1 µF ceramic capacitor** across the 5V and GND rail near the servo and audio amplifiers to prevent ESP32 brownout resets.

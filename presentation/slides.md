# Project Presentation Deck

## Topic: Gesture-Controlled Music Player using Visible Light Communication (VLC)
**Course:** Electronics Workshop (EW) Project  
**Target Event:** Mid-Evaluation & Final Project Defense  

---

## 🩵 Slide 1: Title Slide

* **Project Title:** Gesture-Controlled Music Player using Visible Light Communication (VLC)
* **Course:** Electronics Workshop Project
* **Team Members:**
  * Member 1: Hardware Design, Sensing & Wearable Glove Integration
  * Member 2: Embedded Firmware, VLC Optical Modulation & Audio DSP
  * Member 3: Circuit Assembly, Testing, Calibration & Documentation
* **Department / Semester:** Electronics & Communication Engineering
* **Date:** October 2026
* **Visual Asset:** `[Space reserved for Institutional / Department Logo]`

---

## 🧠 Slide 2: Problem Statement & Interpretation

* **Slide Title:** Problem Statement & System Interpretation
* **Objective:**
  * To design and fabricate an optical, hands-free media playback interface using **Visible Light Communication (VLC)** driven by kinematic hand gestures.
* **Concept Overview:**
  * A wearable glove equipped with flex bend sensors and an MPU-6050 6-DOF IMU (accelerometer + gyroscope) continuously samples hand kinematics.
  * Extracted gestures are processed by an ESP32 controller and converted into high-frequency optical signals emitted by a visible spectrum LED.
  * An optical receiver circuit detects the light pulses, recovers control signals or streaming audio, routes signals via a servo motor, and drives a high-efficiency amplifier speaker setup.
* **Key Use Cases:**
  * **RF-Restricted Environments:** Hospitals, chemical plants, and aircraft cabins where RF radiation (Bluetooth / Wi-Fi) causes electromagnetic interference.
  * **Sanitary / Hands-Free Control:** Sterile medical operating theatres or industrial environments where physical buttons cannot be touched.
  * **Directional Secure Audio:** Optical data does not penetrate opaque walls, creating secure, localized media zones.

---

## 🧩 Slide 3: System Overview & Workflow

* **Slide Title:** End-to-End System Workflow
* **Workflow Architecture:**
  1. **Kinematic Input (Glove Unit):**
     * Flex sensors detect finger curl; MPU-6050 measures pitch (tilt up/down) and roll (tilt left/right).
     * ESP32 runs sensor fusion and debouncing to classify intentional gestures.
  2. **Wireless Transmission Options:**
     * **Mode A (Direct Optical VLC):** Encodes packets or analog audio directly onto a high-flux visible LED.
     * **Mode B (Bluetooth BLE HID):** Emulates Bluetooth keyboard media keys for direct smartphone/laptop control.
  3. **Optical Channel (VLC Link):**
     * Modulated light beam propagates through line-of-sight (LOS) free space.
  4. **Base Station & Audio Delivery:**
     * Photodiode detector captures incoming optical fluctuations.
     * ESP32 decodes commands and actuates an SG90 micro-servo to align optical reception with target speaker zones.
     * Drives dual MAX98357A I²S DAC amplifiers / LM386 analog audio stages to power stereo speakers.

```
+-----------------------------------------------------------------------------------------+
|                                    SYSTEM FLOWCHART                                     |
+-----------------------------------------------------------------------------------------+

  [ Wearable Glove ]
  +------------------+       I2C / ADC       +----------------------+
  | MPU-6050 (Pitch) | --------------------> | ESP32 Transmitter    |
  | 2x Flex Sensors  |                       | (Gesture Classifier) |
  +------------------+                       +----------------------+
                                                        |
                                                        | High-Speed PWM / UART
                                                        v
                                             +----------------------+
                                             | Visible LED Emitter  |
                                             | (465 nm / 650 nm)    |
                                             +----------------------+
                                                        |
                                                        | Line-of-Sight (LOS) Light
                                                        v
  [ Base Audio Station ]
  +----------------------+    Demodulated    +----------------------+
  | Photodiode Receiver  | ----------------> | ESP32 Base Station   |
  | + LM393 Comparator   |      Pulses       | (Command Decoder)    |
  +----------------------+                   +----------------------+
                                                        |
                                        +---------------+---------------+
                                        |                               |
                                        v                               v
                             +--------------------+           +--------------------+
                             | SG90 Micro Servo   |           | Dual MAX98357A /   |
                             | (Directional Aim)  |           | LM386 Audio Amps   |
                             +--------------------+           +--------------------+
                                                                        |
                                                                        v
                                                              +--------------------+
                                                              | Dual 3W Speakers   |
                                                              +--------------------+
```

---

## ⚙️ Slide 4: List of Components

* **Slide Title:** Bill of Materials & Hardware Components

| Component | Qty | Key Function |
|---|:---:|---|
| **ESP32-WROOM-32** | 2–3 | Dual-core processing for sensor fusion, FreeRTOS tasks, and I²S digital audio |
| **MPU-6050 6-DOF IMU** | 1 | Measures pitch, roll, and dynamic tilt gestures over I²C bus |
| **Flex Sensors (2.2")** | 2 | Senses finger curling via resistive voltage divider network |
| **High-Flux Visible LED** | 1–2 | Optical VLC carrier emission (Blue 465 nm / Red 650 nm) |
| **Photodiode Detector** | 1–2 | Silicon photodiode receiver with LM393 high-speed comparator |
| **MAX98357A I²S DAC Amp** | 2 | 3.2W Class-D digital audio amplifiers with hardware shutdown |
| **LM386 Audio Module** | 1 | Analog audio receiver testing and power amplification |
| **SG90 Micro Servo** | 1 | Directs optical emitter or selector toward desired speaker target |
| **Dual 3W 4Ω Speakers** | 2 | Acoustic transducer cones for stereo / zoned sound output |
| **LiPo Battery + TP4056** | 1 | 3.7V 1000 mAh portable power for wearable glove unit |

---

## 🚀 Slide 5: Current Progress & Mid-Evaluation Status

* **Slide Title:** Current Progress & Work Completed
* **Accomplished by Mid-Evaluation:**
  * ✅ **Kinematic Sensing Interfacing:** MPU-6050 and flex sensors fully calibrated with ESP32 ADC and I²C peripherals.
  * ✅ **Gesture Recognition Pipeline:** Complementary filter, debounce timing, and threshold state machines operational for Play/Pause, Volume Ramping, and Track Skip.
  * ✅ **Optical Transmission Channel:** Initial VLC transmitter-receiver link established using high-frequency LED modulation and photodiode demodulation.
  * ✅ **Audio Output Validation:** DAC audio sweep and I²S bus communication verified with clean tone reproduction.
* **Pending Milestones for Final Evaluation:**
  * ⏳ Multi-speaker spatial switching via servo motor.
  * ⏳ Enclosure integration for glove ergonomics and optical lens collimation.
  * ⏳ Optimization of optical reception under direct sunlight/ambient noise.

---

## 🔌 Slide 6: Circuit Diagram & Block Representation

* **Slide Title:** Circuit Architecture & Interconnection
* **Glove Circuit Module:**
  * MPU-6050 wired to `GPIO 21` (SDA) and `GPIO 22` (SCL).
  * Flex sensors wired to `GPIO 34` and `GPIO 35` with 22 kΩ pull-down resistors.
  * Optical LED driven via 2N2222 NPN switching transistor from `GPIO 17`.
* **Receiver Circuit Module:**
  * Photodiode output feeding `GPIO 4` (digital pulse) / `GPIO 34` (analog envelope).
  * Shared I²S bus: `GPIO 22` (BCLK), `GPIO 21` (LRC), and `GPIO 19` (DIN) wired to dual MAX98357A amps.
  * Channel Shutdown: Amp 1 `SD -> GPIO 25`, Amp 2 `SD -> GPIO 26`.
  * SG90 servo PWM signal driven from `GPIO 18`.

---

## 🧾 Slide 7: Summary & Expected Outcome

* **Slide Title:** Project Summary & Final Deliverable
* **Summary:**
  * Demonstrates a novel integration of human body gesture recognition, wireless transmission, and Optical Wireless Communication (OWC).
  * Proves that visible light can serve both as an intuitive line-of-sight command channel and a directional audio carrier.
* **Expected Outcome:**
  * A working, responsive wearable glove that enables seamless hands-free control of an external audio system.
  * Smooth switching between audio outputs and presentation slide navigation without radio-frequency interference.

---

## 🧑‍🏫 Slide 8: Team Members and Acknowledgments

* **Slide Title:** Project Team & Acknowledgments
* **Team Members & Responsibilities:**
  * `[Student Name 1]`: Sensor glove fabrication, flex sensor calibration, wearable mounting.
  * `[Student Name 2]`: VLC optical driver design, photodiode amplifier, FreeRTOS audio programming.
  * `[Student Name 3]`: Power distribution, hardware debugging, test suite, slide presentation.
* **Course Faculty & Mentors:**
  * Electronics Workshop Course Coordinator & Lab Instructors.
* **Acknowledgments:**
  * Grateful appreciation to the department faculty, technical staff, and laboratory facilities for equipment and guidance throughout this project.

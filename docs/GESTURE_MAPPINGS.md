# Gesture Recognition & Command Mappings

This document defines the kinematic gesture dictionary, IMU orientation boundaries, flex sensor threshold values, and corresponding music player control actions.

---

## 1. Kinematic Coordinate System

The MPU-6050 6-DOF IMU is affixed to the back (dorsal side) of the hand:
* **X-Axis (Roll)**: Lateral axis extending across the knuckles. Tilting the hand left or right generates Roll ($\phi$).
* **Y-Axis (Pitch)**: Longitudinal axis extending forward along the middle finger. Tilting the hand up or down generates Pitch ($\theta$).
* **Z-Axis (Yaw)**: Normal axis perpendicular to the back of the hand.

The two flex sensors are oriented longitudinally along:
* **Flex 1**: Index finger proximal and middle phalanges.
* **Flex 2**: Middle finger proximal and middle phalanges.

---

## 2. Gesture Definition Matrix

| Gesture Name | Physical Hand Motion | Sensor Criteria | VLC Opcode | Command Action |
|---|---|---|:---:|---|
| **Fist Clench** | Curl all fingers tightly inward | Flex 1 < 1800 AND Flex 2 < 1800 | `0x10` | **Play / Pause Toggle** |
| **Tilt Forward** | Hand tilted down towards floor | $\text{Pitch} < -30^\circ$, Fingers extended | `0x21` | **Volume Down** ($-5\%$) |
| **Tilt Backward** | Hand tilted up towards ceiling | $\text{Pitch} > +30^\circ$, Fingers extended | `0x20` | **Volume Up** ($+5\%$) |
| **Tilt Left** | Hand rotated counter-clockwise | $\text{Roll} < -35^\circ$ | `0x31` | **Previous Track** |
| **Tilt Right** | Hand rotated clockwise | $\text{Roll} > +35^\circ$ | `0x30` | **Next Track** |
| **Index Point** | Index straight, middle curled | Flex 1 > 2400 AND Flex 2 < 1800 | `0x40` | **Select Speaker A (Left)** |
| **Peace Sign (V)** | Index & middle fingers straight | Flex 1 > 2400 AND Flex 2 > 2400 | `0x41` | **Select Speaker B (Right)** |
| **Flat Palm Sweep** | Open hand sweeping left-to-right | Hand flat ($\vert\text{Pitch}\vert < 15^\circ$), $\text{Roll} \in [0^\circ, 180^\circ]$ | `0x50` | **Aim Servo to Hand Angle** |
| **Neutral / Rest** | Hand relaxed in natural posture | No threshold satisfied | `0x00` | **No Operation (Idle)** |

---

## 3. Sensor Fusion Algorithm (Pitch & Roll Calculation)

The complementary filter blends fast gyroscope responsiveness with drift-free accelerometer gravitational vectors:

$$\theta_{\text{acc}} = \text{atan2}(-A_x, \sqrt{A_y^2 + A_z^2}) \times \frac{180}{\pi}$$

$$\phi_{\text{acc}} = \text{atan2}(A_y, A_z) \times \frac{180}{\pi}$$

$$\theta_t = \alpha \cdot (\theta_{t-1} + G_y \cdot \Delta t) + (1 - \alpha) \cdot \theta_{\text{acc}}$$

$$\phi_t = \alpha \cdot (\phi_{t-1} + G_x \cdot \Delta t) + (1 - \alpha) \cdot \phi_{\text{acc}}$$

* Filter coefficient $\alpha = 0.96$
* Sampling loop interval $\Delta t = 20\text{ ms}$ ($50\text{ Hz}$)

---

## 4. State Machine & Debouncing

```
       +--------------+
       |     IDLE     | <-------------------------+
       +--------------+                           |
              |                                   |
              | Gesture Threshold Crossed         |
              v                                   |
       +--------------+                           |
       |  DEBOUNCE    | (Check stable for 60 ms)  |
       +--------------+                           |
              |                                   |
              | Confirmed Valid                   | Refractory Timeout
              v                                   | (300 ms elapsed)
       +--------------+                           |
       | TRANSMITTING |                           |
       |  (VLC Burst) |                           |
       +--------------+                           |
              |                                   |
              +-----------------------------------+
```

* **Stability Window**: A gesture candidate must maintain threshold compliance for at least **60 ms** (3 consecutive sensor cycles) before triggering.
* **Refractory Cooldown**: After any command is emitted, a **300 ms** refractory lockout prevents erratic duplicate triggers during natural hand reset.

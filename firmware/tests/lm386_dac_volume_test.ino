#include <driver/dac.h>
#include <math.h>

#define DAC_PIN DAC_CHANNEL_1   // GPIO25

void setup() {
  Serial.begin(115200);
  Serial.println("Starting LM386 Volume Sweep Test...");
  
  dac_output_enable(DAC_PIN);
}

void loop() {
  const int freq = 1000;             // 1 kHz test tone
  const int sampleRate = 20000;      // DAC update rate
  const float step = (2 * M_PI * freq) / sampleRate;

  static float phase = 0;

  // Fade volume UP
  for (int vol = 0; vol <= 255; vol++) {
    Serial.print("VOLUME UP: ");
    Serial.println(vol);

    for (int i = 0; i < 40; i++) {      // small block so Serial stays readable
      int val = (sin(phase) * (vol / 2)) + 128;
      dac_output_voltage(DAC_PIN, val);

      phase += step;
      if (phase >= 2 * M_PI) phase -= 2 * M_PI;

      delayMicroseconds(50);
    }
  }

  // Fade volume DOWN
  for (int vol = 255; vol >= 0; vol--) {
    Serial.print("VOLUME DOWN: ");
    Serial.println(vol);

    for (int i = 0; i < 40; i++) {
      int val = (sin(phase) * (vol / 2)) + 128;
      dac_output_voltage(DAC_PIN, val);

      phase += step;
      if (phase >= 2 * M_PI) phase -= 2 * M_PI;

      delayMicroseconds(50);
    }
  }
}
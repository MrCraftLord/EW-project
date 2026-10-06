// VLC Receiver for ESP32
// Reads photodiode AC signal on ADC pin (GPIO34)
// Demodulates + smooths it
// Outputs audio on DAC GPIO25 (DAC1)

#include <driver/adc.h>
#include <driver/dac.h>

#define ADC_PIN 34      // Your photodiode input
#define DAC_PIN 25      // Output to LM386

// Low-pass filter coefficient for smoothing the envelope
// Increase for smoother audio, decrease for sharper audio
float alpha = 0.05;  

void setup() {
  Serial.begin(115200);

  // Initialize ADC
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  
  // ADC1_CHANNEL_6 = GPIO34
  
  // Initialize DAC
  dac_output_enable(DAC_CHANNEL_1);  // GPIO25
}

void loop() {
  // 1) Read raw ADC sample
  int raw = adc1_get_raw(ADC1_CHANNEL_6);

  // The ADC reading is biased around 1.65V.
  // Remove the DC offset by subtracting a midpoint (approx. 2048).
  int centered = raw - 2048;

  // 2) Envelope detection (absolute value)
  int envelope = abs(centered);

  // 3) Low-pass filter to smooth audio
  static float smooth = 0;
  smooth = smooth + alpha * (envelope - smooth);

  // Map filtered envelope to 0–255 for DAC
  int audioOut = constrain((int)(smooth / 8), 0, 255);

  // 4) Output to DAC -> LM386 -> speaker
  dac_output_voltage(DAC_CHANNEL_1, audioOut);

  // Delay to get ~20 kHz sampling rate
  delayMicroseconds(50);
}
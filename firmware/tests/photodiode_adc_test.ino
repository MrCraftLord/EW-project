#include <driver/adc.h>

#define PD_PIN ADC1_CHANNEL_6   // GPIO34

void setup() {
  Serial.begin(115200);
  
  // Configure ADC width and attenuation
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(PD_PIN, ADC_ATTEN_DB_11); 
  // 11dB gives full 0–3.3V range
}

void loop() {
  int raw = adc1_get_raw(PD_PIN);

  Serial.println(raw);   // print brightness level (0–4095)

  delay(5);  // small delay for readability
}
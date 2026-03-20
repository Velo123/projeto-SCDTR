#include <Arduino.h>

#define LED_PIN 9
#define LDR_PIN 26  // ADC input from LDR voltage divider

bool ledActivated = false;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 255);
  
  delay(7000);
}

void loop() {

  unsigned long now = millis(); // Convert to milliseconds with microsecond precision

  Serial.print(now);
  Serial.print(',');
  Serial.print(ledActivated);
  Serial.print(',');
  Serial.println(analogRead(LDR_PIN));
  
  if (now>10000 && !ledActivated)
  {
    analogWrite(LED_PIN,0.8*255);
    ledActivated = true;
  }
}

/*
   Distributed Real-Time Control Systems Project
   Luminaire Local Controller
   Raspberry Pi Pico
   Sampling: 100 Hz
*/

#include <Arduino.h>

// =====================
// PIN DEFINITIONS
// =====================

#define PWM_PIN 9
#define LDR_PIN 26

// =====================
// MODE SELECTION
// =====================

#define MODE_SWEEP 0
#define MODE_CALIBRATE_B 1

int MODE = 0;   // <<< ALTERAR AQUI

// =====================
// SYSTEM PARAMETERS
// =====================

#define TS 0.01
#define PWM_MAX 255
#define R_FIXED 10000.0

// =====================
// LDR CALIBRATION
// =====================

float m = -0.8;
float b = 6.1;

float lux = 0;

// =====================
// TIMER
// =====================

unsigned long lastTime = 0;

// =====================
// PWM
// =====================

void setPWM(float duty)
{
  if (duty < 0) duty = 0;
  if (duty > 1) duty = 1;

  analogWrite(PWM_PIN, duty * PWM_MAX);
}



// =====================
// MOVING AVERAGE FILTER
// =====================

float luxSum = 0.0;
int luxsumADC = 0;
int sampleCount = 0;
int sampleCountADC = 0;


void addSampleToBufferADC() {

  luxsumADC += analogRead(LDR_PIN);
  sampleCountADC++;
}

float getMovingAverageADC() {
  if (sampleCountADC==0)
  {
    addSampleToBufferADC();
  }
  
  float average = (sampleCountADC > 0) ? luxsumADC / sampleCountADC : 0.0;
  
  // Reset for next period
  luxsumADC = 0.0;
  sampleCountADC = 0;
  
  return average;
}

float getavgvoltage(float ADCavg) {

  float v_out = (ADCavg * 3.3) / 4095.0;

  return v_out;
}

float getLDRresistance(float v_out) {

  float r_ldr = (R_FIXED * (3.3 - v_out)) / v_out;

  return r_ldr;
}

float getavglux(float ADCavg) {

  float v_out = getavgvoltage(ADCavg);
  float r_ldr = getLDRresistance(v_out);
  float log10_ldr = log10(r_ldr);
  float avglux = pow(10, (log10_ldr - b) / m);
  return avglux;
}

// =====================
// SWEEP STORAGE
// =====================

float luxes[101];
int sweepIndex = 0;

void setup()
{
  Serial.begin(115200);
  delay(5000);
  analogReadResolution(12);

  pinMode(PWM_PIN, OUTPUT);

  analogWrite(PWM_PIN, 0);

}


void loop()
{
  if (MODE == MODE_SWEEP)
  {

    unsigned long now = millis();

    if (now - lastTime >= 1000)
    {
      lastTime = now;

      for (int i = 0; i < 100; i++)
      {
        addSampleToBufferADC();
        delay(10);
      }
      lux = getavglux(getMovingAverageADC());
      
      if (sweepIndex <= 100)
      {
        float duty = sweepIndex * 0.01;
        setPWM(duty);
        luxes[sweepIndex] = lux;
        Serial.print(duty, 2);
        Serial.print(",");
        Serial.println(lux);
        sweepIndex++;
      }
      else
      {
        for (int i = 0; i <= 100; i++)
        {
          Serial.print(i * 0.01, 2);
          Serial.print(",");
          Serial.println(luxes[i]);
        }
        while (1);
      }
    }
  }


// =====================================================
// MODE 1 : CALCULATE b FOR 500 LUX
// =====================================================

  if (MODE == MODE_CALIBRATE_B)
  {

    unsigned long now = millis();
    if (now - lastTime >= 1000)
    {
      lastTime = now;
      float r_ldr = 0;
      // média de resistência
      for (int i = 0; i < 100; i++)
      {
        addSampleToBufferADC();
        delay(10);
      }

      r_ldr = getLDRresistance(getavgvoltage(getMovingAverageADC()));
      
      float lux_ref = 500.0;
      float b_calculated = log10(r_ldr) - m * log10(lux_ref);
      Serial.print("R_LDR = ");
      Serial.println(r_ldr);
      Serial.print("Calculated b = ");
      Serial.println(b_calculated, 6);

      while(1);
    }
  }
}
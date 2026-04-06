#include <Arduino.h>
#include "cfg.h"
#include "pid.h"
#include "serial.h"
#include "canv2.h"
#include "shared.h"
#include <string.h>

/*
   Distributed Real-Time Control Systems Project
   Luminaire Local Controller - Complete Version
   Platform: Raspberry Pi Pico (Arduino IDE)
   Sampling: 100 Hz
*/
const uint8_t interruptPin {0};
PID controller(0.02, 0.1, 0.0,  // Kp, Ki, Kd
               1.0, 0.0,       // beta, gamma
               TS,           // Ts
               1.0,            // Kff
               true,           // use feedforward
               true);

static void applyPendingCommands() {
  PendingCommands pending = {};

  critical_section_enter_blocking(&gStateLock);
  // Copy and clear the full pending struct so all fields are handled.
  memcpy(&pending, (const void*)&gPending, sizeof(PendingCommands));
  memset((void*)&gPending, 0, sizeof(PendingCommands));

  if (pending.hasDuty) {
    gOutputs.duty = pending.newDuty;
  }
  if (pending.hasReferenceLux) {
    gInputs.referenceLux = pending.newReferenceLux;
  }
  if (pending.hasOccupancyState) {
    gInputs.occupancyState = pending.newOccupancyState;
    // Automatically update reference lux based on new occupancy state
    if (pending.newOccupancyState == 'h') {
      gInputs.referenceLux = gInputs.refHigh;
    } else if (pending.newOccupancyState == 'l') {
      gInputs.referenceLux = gInputs.refLow;
    } else {
      gInputs.referenceLux = gInputs.refOccupied;  // 'o' state
    }
  }
  if (pending.hasAntiWindupEnabled) {
    gInputs.antiWindupEnabled = pending.newAntiWindupEnabled;
  }
  if (pending.hasFeedbackEnabled) {
    gInputs.feedbackEnabled = pending.newFeedbackEnabled;
  }
  if (pending.hasManualOverride) {
    gInputs.manualOverride = pending.newManualOverride;
  }
  if (pending.haspwm) {
    for (int i = 0; i < 3; i++) {
      gInputs.pwm[i] = pending.newpwm[i];
    }
  }
  if (pending.hasRefOccupied) {
    gInputs.refOccupied = pending.newRefOccupied;
  }
  if (pending.hasRefLow) {
    gInputs.refLow = pending.newRefLow;
  }
  if (pending.hasRefHigh) {
    gInputs.refHigh = pending.newRefHigh;
  }
  critical_section_exit(&gStateLock); 
}

//Core 0: Control loop and sensor reading
unsigned long lastTimePID = 0;

void setup() {
  initSharedState();
  analogReadResolution(12);
  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, 0);
}

void loop() {
  unsigned long now = millis();
  
  if (!calibrating && now - lastTimePID >= 10) { // 10ms control period
    unsigned long elapsedMs = now - lastTimePID;
    lastTimePID = now;
    applyPendingCommands();

    float adcAvg = getMovingAverageADC();
    float lux = getavglux(adcAvg);
    float duty = gOutputs.duty;
  
    // Skip PID computation if manual override is active (use preset duty cycle instead).
    if (!gInputs.manualOverride) {
      duty = controller.compute(gInputs.referenceLux, lux);
    }
    setPWM(duty);

    float ldrVoltage = getavgvoltage(adcAvg);
    float ldrResistance = getLDRresistance(ldrVoltage);
    constexpr float MAX_DESK_POWER_W = 0.096f; // 96 mW at 100% duty cycle
    float instantPower = duty * MAX_DESK_POWER_W;
    float accumulatedEnergy = gOutputs.accumulatedEnergy + instantPower * (static_cast<float>(elapsedMs) / 1000.0f);
    float averageVisibilityError = compute_avg_visibility_err(gInputs.referenceLux, lux);
    float averageFlicker = compute_avg_flicker(duty, gInputs.referenceLux);

    critical_section_enter_blocking(&gStateLock);
    gOutputs.timestampMs = now;
    gOutputs.duty = duty;
    gOutputs.luxMeasured = lux;
    gOutputs.ldrVoltage = ldrVoltage;
    gOutputs.ldrResistance = ldrResistance;
    gOutputs.instantPower = instantPower;
    gOutputs.accumulatedEnergy = accumulatedEnergy;
    gOutputs.averageVisibilityError = averageVisibilityError;
    gOutputs.averageFlicker = averageFlicker;

    // Store last-minute history in a circular buffer.
    gHistory.timestampMs[gHistory.head] = now;
    gHistory.pwmDuty[gHistory.head] = duty;
    gHistory.illuminanceLux[gHistory.head] = lux;
    gHistory.head = static_cast<uint16_t>((gHistory.head + 1) % HISTORY_BUFFER_SAMPLES);
    if (gHistory.count < HISTORY_BUFFER_SAMPLES) {
      gHistory.count++;
    }

    critical_section_exit(&gStateLock);
  }
  else {
    // Collect samples between control periods
    addSampleToBufferADC();
  }
}

//Core 1: Communications
volatile bool got_irq {false};

//the interrupt service routine for core 1
void read_interrupt(uint gpio, uint32_t events) {
  got_irq = true;  
}

unsigned long lastTimePWM = 0;
unsigned long lastTimeStream = 0;

void setup1() { 
  Serial.begin(115200); 
  Serial.setTimeout(10);
  init_can();
  gpio_set_irq_enabled_with_callback(interruptPin,GPIO_IRQ_EDGE_FALL,true,&read_interrupt ); 
}

void loop1() {
  unsigned long now = millis();
  if (got_irq){
    got_irq = false; // Reset flag
    // Process CAN message
    processirq();
    can0.clearRXnOVRFlags();  
    can0.clearInterrupts();
  }
  if (Serial.available() > 0) {
    handleSerial();
  }
  if (calibrating!=true)
  {
    if (now - lastTimeStream >= 100) {
      lastTimeStream = now;
      print_to_serial();
    }

    if (now - lastTimePWM >= 5) { // 20ms CAN update
      lastTimePWM = now;
      // Send PWM value over CAN
      critical_section_enter_blocking(&gStateLock);
      float dutyToSend = gOutputs.duty;
      critical_section_exit(&gStateLock);
      encode_and_send(INTERNAL,_luminaireId,0,CAN_MSG_PWM,dutyToSend);
    }
  }
  


  
}


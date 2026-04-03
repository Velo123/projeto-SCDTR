#pragma once
#include <Arduino.h>
#include <pico/critical_section.h>

struct ControlInputs {
  float referenceLux;
  char occupancyState;
  bool antiWindupEnabled;
  bool feedbackEnabled;
  bool manualOverride;
  float pwm[3]; // for future use: store PWM values of other luminaires
  float refOccupied;  // reference illuminance for 'o' (occupied) state
  float refLow;       // reference illuminance for 'l' (low) state
  float refHigh;      // reference illuminance for 'h' (high) state
};

struct ControlOutputs {
  uint32_t timestampMs;
  float duty;
  float luxMeasured;
  float ldrVoltage;
  float ldrResistance;
};

// 60 s history with 10 ms sampling period -> 6000 samples.
constexpr uint16_t HISTORY_BUFFER_SAMPLES = 6000;

struct HistoryBuffers {
  uint32_t timestampMs[HISTORY_BUFFER_SAMPLES];
  float pwmDuty[HISTORY_BUFFER_SAMPLES];
  float illuminanceLux[HISTORY_BUFFER_SAMPLES];
  uint16_t head;
  uint16_t count;
};

struct PendingCommands {
  bool hasDuty;
  float newDuty;

  bool hasReferenceLux;
  float newReferenceLux;

  bool hasOccupancyState;
  char newOccupancyState;

  bool hasAntiWindupEnabled;
  bool newAntiWindupEnabled;

  bool hasFeedbackEnabled;
  bool newFeedbackEnabled;

  bool hasManualOverride;
  bool newManualOverride;

  bool haspwm;
  float newpwm[3];

  bool hasRefOccupied;
  float newRefOccupied;

  bool hasRefLow;
  float newRefLow;

  bool hasRefHigh;
  float newRefHigh;
};

extern volatile ControlInputs gInputs;
extern volatile ControlOutputs gOutputs;
extern volatile HistoryBuffers gHistory;
extern volatile PendingCommands gPending;
extern critical_section_t gStateLock;

extern bool waiting_can;

void initSharedState();
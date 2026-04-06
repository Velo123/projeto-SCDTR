#include "shared.h"

volatile ControlInputs gInputs = {20.0f, 'o', true, true, false, {0.0f, 0.0f, 0.0f}, 20.0f, 20.0f, 20.0f};
volatile ControlOutputs gOutputs = {0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
volatile HistoryBuffers gHistory = {{0}, {0.0f}, {0.0f}, 0, 0};
volatile PendingCommands gPending = {};
critical_section_t gStateLock;
bool waiting_can = false;

float measuredLux0[10];
float measuredLux1[10];
float measuredLux2[10];

bool calibrating = false;
int calibrating_luminaireId=0;

void initSharedState() {
  critical_section_init(&gStateLock);
}






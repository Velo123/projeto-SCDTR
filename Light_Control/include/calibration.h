#include "canv2.h"
#include "shared.h"
#include "pid.h"


extern float measuredLux0[10];
extern float measuredLux1[10];
extern float measuredLux2[10];


void calibrate();
void updateCalibration(float src, float pwm);
float calibrateSystem(const float* measuredLux);
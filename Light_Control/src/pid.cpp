#include "pid.h"

// =====================
// GLOBAL VARIABLES DEFINITIONS
// =====================
float luxSum = 0.0;
int luxsumADC = 0;
int sampleCount = 0;
int sampleCountADC = 0;

// =====================
// LUX READING
// =====================

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


void setPWM(float dutyCycle) {

  if (dutyCycle > 1.0) dutyCycle = 1.0;
  if (dutyCycle < 0.0) dutyCycle = 0.0;

  analogWrite(PWM_PIN, dutyCycle * PWM_MAX);
}


// =====================
// PID CLASS IMPLEMENTATIONS
// =====================
PID::PID(float kp, float ki, float kd,
        float swbeta, float g,
        float ts,
        float kff,
    bool feedBackOn,
    bool antiWindupOn)
{
    Kp = kp; Ki = ki; Kd = kd;
    beta = swbeta; gamma = g;
    Kff = kff;
    Ts = ts; 

    integrator = 0.0;
    previousMeasurement = 0.0;
    previousReference = 0.0;

    outputMin = 0.0;
    outputMax = 1.0;

    for (int i = 0; i < 3; i++) {
        luminaireGain[i] = 0.0f;
    }

    FBon = feedBackOn;
    AWon = antiWindupOn;
}

void PID::reset() {
    integrator = 0.0;
    previousMeasurement = 0.0;
    previousReference = 0.0;
}

float PID::compute(float reference, float measurement) {

    ControlInputs inputs;
    float gains[3] = {0.0f, 0.0f, 0.0f};
    critical_section_enter_blocking(&gStateLock); // make a local copy to minimize time in critical section
        inputs.referenceLux = gInputs.referenceLux;
        inputs.antiWindupEnabled = gInputs.antiWindupEnabled;
        inputs.feedbackEnabled = gInputs.feedbackEnabled;
        inputs.luminanceControlEnabled = gInputs.luminanceControlEnabled;  
        inputs.occupancyState = gInputs.occupancyState;
        for (int i = 0; i < 3; i++) {
            inputs.pwm[i] = gInputs.pwm[i];
            gains[i] = luminaireGain[i];
        } 
    critical_section_exit(&gStateLock);


    float external_lux=0.0;

    if(_luminaireId == 0){
        external_lux = inputs.pwm[1]*gains[1] + inputs.pwm[2]*gains[2];
    }
    else if(_luminaireId == 1){
        external_lux = inputs.pwm[0]*gains[0] + inputs.pwm[2]*gains[2];
    }
    else if(_luminaireId == 2){
        external_lux = inputs.pwm[0]*gains[0] + inputs.pwm[1]*gains[1];
    }
    


    if (!FBon) {
        // modo feedforward (apenas ação proporcional ao setpoint) 
        float output = Kff * reference - external_lux; // feedforward puro
        // saturação
        if (output > outputMax) output = outputMax;
        if (output < outputMin) output = outputMin;
        return output;
    }

    // modo feedback (PID completo)
    float error = reference - measurement;
    
    float P = Kp * (beta * reference - measurement);
    float D = Kd * ((gamma * reference - measurement) -
                    (gamma * previousReference - previousMeasurement)) / Ts;

    if (AWon) {
        if (!((P + integrator + D >= outputMax && error > 0) ||
              (P + integrator + D <= outputMin && error < 0))) 
        {
            integrator += Ki * Ts * error;
        }
    } else {
        integrator += Ki * Ts * error;
    }

    float output = P + integrator + D;

    // saturação
    if (output > outputMax) output = outputMax;
    if (output < outputMin) output = outputMin;

    previousMeasurement = measurement;
    previousReference = reference;

    return output;
}

void PID::setOutputLimits(float minVal, float maxVal) {
    outputMin = minVal;
    outputMax = maxVal;
}

void PID::setsetpointWeighting(float swbeta, float g) {
    beta = swbeta;
    gamma = g;
}

bool PID::setWeight(Weight weight, float value) {
    switch (weight) {
        case KP:
            Kp = value;
            return true;
        case KI:
            Ki = value;
            return true;
        case KD:
            Kd = value;
            return true;
        case BETA:
            beta = value;
            return true;
        case GAMMA:
            gamma = value;
            return true;
        case KFF:
            Kff = value;
            return true;
        case SAMPLE_TIME:
            if (value <= 0.0f) {
                return false;
            }
            Ts = value;
            return true;
        default:
            return false;
    }
}

float PID::getWeight(Weight weight) const {
    switch (weight) {
        case KP:
            return Kp;
        case KI:
            return Ki;
        case KD:
            return Kd;
        case BETA:
            return beta;
        case GAMMA:
            return gamma;
        case KFF:
            return Kff;
        case SAMPLE_TIME:
            return Ts;
        default:
            return 0.0f;
    }
}

bool PID::setLuminaireGain(uint8_t luminaireId, float gain) {
    if (luminaireId >= 3) {
        return false;
    }

    critical_section_enter_blocking(&gStateLock);
    luminaireGain[luminaireId] = gain;
    critical_section_exit(&gStateLock);
    return true;
}

float PID::getLuminaireGain(uint8_t luminaireId) const {
    if (luminaireId >= 3) {
        return 0.0f;
    }

    float gain;
    critical_section_enter_blocking(&gStateLock);
    gain = luminaireGain[luminaireId];
    critical_section_exit(&gStateLock);
    return gain;
}

void PID::setModeFeedforward(bool enable) {
    FBon = enable;
}

void PID::setAntiWindup(bool enable) {
    AWon = enable;
}
#include "pid.h"
#include <cmath>

// =====================
// GLOBAL VARIABLES DEFINITIONS
// =====================
float luxSum = 0.0;
int luxsumADC = 0;
int sampleCount = 0;
int sampleCountADC = 0;


float instant_Pwr = 0.0;

float energy_sum = 0.0;
unsigned long energy_sample_count = 0;

float avg_visibility_err = 0.0;
float visibility_err_sum = 0.0;
unsigned long visibility_err_sample_count = 0;

float avg_flicker = 0.0;
float flicker_sign_ = 0.0;
float flicker_sum = 0.0;
unsigned long flicker_sample_count = 0;
float prev_duty_k1 = 0.0f;
float prev_duty_k2 = 0.0f;
float prev_ref_flicker = 0.0f;
bool flicker_initialized = false;

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

    // Protect against invalid controller outputs.
    if (!isfinite(dutyCycle)) dutyCycle = 0.0f;

  if (dutyCycle > 1.0) dutyCycle = 1.0;
  if (dutyCycle < 0.0) dutyCycle = 0.0;

  analogWrite(PWM_PIN, dutyCycle * PWM_MAX);
}

void reset_flicker_metrics() {
    avg_flicker = 0.0f;
    flicker_sign_ = 0.0f;
    flicker_sum = 0.0f;
    flicker_sample_count = 0;
    prev_duty_k1 = 0.0f;
    prev_duty_k2 = 0.0f;
    prev_ref_flicker = 0.0f;
    flicker_initialized = false;
}

float compute_avg_flicker(float dutyCycle, float referenceLux){
    if (!flicker_initialized) {
        prev_duty_k2 = dutyCycle;
        prev_duty_k1 = dutyCycle;
        prev_ref_flicker = referenceLux;
        flicker_initialized = true;
        return 0.0f;
    }

    // Exclude transient periods caused by explicit reference changes.
    if (referenceLux != prev_ref_flicker) {
        prev_ref_flicker = referenceLux;
        prev_duty_k2 = dutyCycle;
        prev_duty_k1 = dutyCycle;
        return avg_flicker;
    }

    float delta_k = dutyCycle - prev_duty_k1;
    float delta_km1 = prev_duty_k1 - prev_duty_k2;

    float f_k = 0.0f;
    if ((delta_k * delta_km1) < 0.0f) {
        f_k = fabs(delta_k) + fabs(delta_km1);
    }

    flicker_sign_ = f_k;
    flicker_sum += f_k;
    flicker_sample_count++;
    avg_flicker = flicker_sum / static_cast<float>(flicker_sample_count);

    prev_duty_k2 = prev_duty_k1;
    prev_duty_k1 = dutyCycle;
    return avg_flicker;
}

float compute_avg_energy(){
    return 0;
}


float compute_avg_visibility_err(float referenceLux, float measuredLux){
    float visibilityError = referenceLux - measuredLux;
    if (visibilityError < 0.0f) {
        visibilityError = 0.0f;
    }

    visibility_err_sum += visibilityError;
    visibility_err_sample_count++;
    avg_visibility_err = visibility_err_sum / visibility_err_sample_count;
    return avg_visibility_err;
}

float getInstantPower() {
    float instantPower = 0.0f;

    critical_section_enter_blocking(&gStateLock);
    instantPower = gOutputs.instantPower;
    critical_section_exit(&gStateLock);

    return instantPower;
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

    FBon = feedBackOn;
    AWon = antiWindupOn;
}

void PID::reset() {
    integrator = 0.0;
    previousMeasurement = 0.0;
    previousReference = 0.0;
}


float PID::compute(float reference, float measurement) {

    float external_lux_compensation=getExternalLuminance();  
    critical_section_enter_blocking(&gStateLock);   
    Kff = gInputs.gain[_luminaireId];
    FBon = gInputs.feedbackEnabled;
    AWon = gInputs.antiWindupEnabled;
    critical_section_exit(&gStateLock);


    if (!FBon) {
        // modo feedforward (apenas ação proporcional ao setpoint) 
        float output = reference/Kff;// - getPWMcompensation(); // feedforward controller
        // saturação
        if (output > outputMax) output = outputMax;
        if (output < outputMin) output = outputMin;
        return output;
    }

    // modo feedback (PID completo)
    float error = reference - measurement;
    //Serial.print("Reference: "); Serial.print(reference); Serial.print(" Measured: "); Serial.print(measurement); Serial.print(" Compensation: "); Serial.println(external_lux_compensation);
    float P = Kp * (reference - measurement-external_lux_compensation); // considerando ganho das outras luminárias como perturbação
    float D = Kd * ((reference - measurement) - (previousReference - previousMeasurement)) / Ts;

    if (AWon) {
        if (!((P + integrator + D >= outputMax && error > 0) ||
              (P + integrator + D <= outputMin && error < 0))) 
        {
            integrator += Ki * Ts * error;
        }
    } else {
        integrator += Ki * Ts * error;
    }

    float output = (P + integrator + D); // considerando ganho das outras luminárias como perturbação

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

float PID::getExternalLuminance() {
    ControlInputs inputs;
    float pwm[3],luminaireGain[3];
    critical_section_enter_blocking(&gStateLock);
    pwm[0] = gInputs.pwm[0];
    pwm[1] = gInputs.pwm[1];
    pwm[2] = gInputs.pwm[2];
    luminaireGain[0] = gInputs.gain[0];
    luminaireGain[1] = gInputs.gain[1];
    luminaireGain[2] = gInputs.gain[2];
    critical_section_exit(&gStateLock);

    float external_lux_compensation=0.0;

    if(_luminaireId == 0){
        external_lux_compensation = pwm[1]*luminaireGain[1] + pwm[2]*luminaireGain[2];
    }
    else if(_luminaireId == 1){
        external_lux_compensation = pwm[0]*luminaireGain[0] + pwm[2]*luminaireGain[2];
    }
    else if(_luminaireId == 2){
        external_lux_compensation = pwm[0]*luminaireGain[0] + pwm[1]*luminaireGain[1];
    }

    return external_lux_compensation;
}

float PID::getPWMcompensation() {
    critical_section_enter_blocking(&gStateLock);
    float luminaireGain = gInputs.gain[_luminaireId];
    critical_section_exit(&gStateLock);

    if (!isfinite(luminaireGain) || fabs(luminaireGain) < 1e-6f) {
        return 0.0f;
    }

    float externalLuminance = getExternalLuminance();
    if (!isfinite(externalLuminance)) {
        return 0.0f;
    }

    float pwm_compensation = externalLuminance / luminaireGain;
    if (!isfinite(pwm_compensation) || pwm_compensation < 0.0f || pwm_compensation > 1.0f)
    {
        return 0.0f;
    } 
    
    return pwm_compensation;
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

void PID::setModeFeedforward(bool enable) {
    FBon = enable;
}

void PID::setAntiWindup(bool enable) {
    AWon = enable;
}



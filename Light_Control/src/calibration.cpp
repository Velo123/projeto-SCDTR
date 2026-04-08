#include "calibration.h"


float measuredLux0[10];
float measuredLux1[10];
float measuredLux2[10];

static float readStableLux(uint16_t settleMs = 250, uint8_t warmupReads = 3, uint8_t avgReads = 4) {
    delay(settleMs);

    // Discard transient samples after each PWM change.
    for (uint8_t i = 0; i < warmupReads; ++i) {
        getavglux(getMovingAverageADC());
        delay(30);
    }

    float sumLux = 0.0f;
    for (uint8_t i = 0; i < avgReads; ++i) {
        sumLux += getavglux(getMovingAverageADC());
        delay(30);
    }

    return sumLux / static_cast<float>(avgReads);
}

void calibrate() {
  
    //Send message that this node wants to calibrate
    int targetLuminaireId = 0;
    critical_section_enter_blocking(&gStateLock);
    targetLuminaireId = calibrating_luminaireId;
    critical_section_exit(&gStateLock);

    if(targetLuminaireId!=_luminaireId){
        return; // Only calibrate if this is the intended luminaire
    }

    Serial.print("Starting calibration for luminaire ");Serial.print(targetLuminaireId);Serial.println("...");
    getavglux(getMovingAverageADC());
    setPWM(0.0f);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0);
    delay(3000); // Wait for any ongoing operations to settle in core 0 before starting calibration
    
    for (int i = 0; i < 11; ++i) {
        Serial.print("Setting PWM to "); Serial.print(i * 10); Serial.println("%");
        setPWM(i/10.0f);
        getavglux(getMovingAverageADC());
        delay(1000);
        encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, i/10.0f);
        delay(1000);
        if(i==0){
            delay(1000); // Extra delay at 0% to allow for sensor stabilization without light output
            continue;
        } // Skip reading lux at 0% duty cycle to avoid log(0) issues in gain calculation
        if (_luminaireId==0) {
            measuredLux0[i-1] = getavglux(getMovingAverageADC());
        }else if (_luminaireId==1) {
            measuredLux1[i-1] = getavglux(getMovingAverageADC());
        }else if (_luminaireId==2) {
            measuredLux2[i-1] = getavglux(getMovingAverageADC());
        }
    }
    // Ensure local gain computation does not depend on CAN self-reception.
    // If self-reception already processed pwm=1.0, state may have advanced and we skip duplicate completion.
    critical_section_enter_blocking(&gStateLock);
    targetLuminaireId = calibrating_luminaireId;
    critical_section_exit(&gStateLock);
    if (targetLuminaireId == _luminaireId) {
        updateCalibration(_luminaireId, 1.0f);
    }
    return;
}

void updateCalibration(float src, float pwm) {

    const int srcId = static_cast<int>(src);
    if (srcId < 0 || srcId > MAXID) {
        return;
    }
    int pwmval = static_cast<int>(lroundf(pwm * 10.0f)) ;


    // Start of another luminaire calibration: keep local LED off and only sample lux on next steps.
    if (srcId != _luminaireId) {
        setPWM(0.0f);
        critical_section_enter_blocking(&gStateLock);
        gOutputs.duty = 0.0f;
        critical_section_exit(&gStateLock);
    }

    bool shouldStartCalibration = false;
    int expectedSrcId = 0;
    critical_section_enter_blocking(&gStateLock);
    expectedSrcId = calibrating_luminaireId;

    // Ignore delayed/out-of-order CAN frames to keep calibration sequence deterministic.
    if (srcId != expectedSrcId) {
        critical_section_exit(&gStateLock);
        return;
    }

    if (!calibrating && _luminaireId == 0 && !startupCalibrationPending) {
        // Startup calibration already finished; ignore delayed/duplicate frames.
        critical_section_exit(&gStateLock);
        return;
    }

    if (!calibrating && _luminaireId == 0 && startupCalibrationPending)
    {
        calibrating = true;
        startupCalibrationPending = false;
        shouldStartCalibration = true;
    } else {
        calibrating = true; // Set calibrating flag to indicate calibration is in progress
    }
    critical_section_exit(&gStateLock);

    if (shouldStartCalibration)
    {
        calibrate(); // Start calibration if not already calibrating and this is the first luminaire
    }

    if (pwmval < 1 || pwmval > 10) {
        return;
    }

    if(srcId == 0){
        measuredLux0[pwmval-1] = getavglux(getMovingAverageADC());
    }else if(srcId == 1){
        measuredLux1[pwmval-1] = getavglux(getMovingAverageADC());
    }else if(srcId == 2){
        measuredLux2[pwmval-1] = getavglux(getMovingAverageADC());
    }

    if (pwmval==10){
        float slope = 0.0f;

        if (srcId == 0) {
            slope = calibrateSystem(measuredLux0);
        } else if (srcId == 1) {
            slope = calibrateSystem(measuredLux1);
        } else if (srcId == 2) {
            slope = calibrateSystem(measuredLux2);
        }

        critical_section_enter_blocking(&gStateLock);
        gInputs.gain[srcId] = slope;
        critical_section_exit(&gStateLock);

        critical_section_enter_blocking(&gStateLock);
        calibrating_luminaireId = srcId + 1; // Set to a value that indicates calibration is done for this luminaire
        if(calibrating_luminaireId>MAXID){
            calibrating_luminaireId=0; // Reset for potential future calibrations
            calibrating = false; // Clear calibrating flag after all luminaires have been calibrated
            startupCalibrationPending = false;
        }else if(calibrating_luminaireId==_luminaireId){
            critical_section_exit(&gStateLock);
            calibrate(); // Start calibration for the next luminaire
            return;
        }
        critical_section_exit(&gStateLock);

    }
    return;
}

float calibrateSystem(const float* measuredLux) {

    const int numPoints = 9;
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumXY = 0.0f;
    float sumX2 = 0.0f;
    if (debug)
    {
        Serial.println("Calibration data points:");
        Serial.println("PWM, Lux");
        Serial.println("----------------");
         for (int i = 1; i < 10; i++)
        {
            Serial.print(0.1f * (i + 1)); Serial.print(", "); Serial.println(measuredLux[i]);
        }
    } 

    for (int i = 1; i < numPoints; ++i) {
      const float pwm = 0.1f * (i + 1);  // 0.1, 0.2, ..., 1.0
      const float lux = measuredLux[i];

      sumX += pwm;
      sumY += lux;
      sumXY += pwm * lux;
      sumX2 += pwm * pwm;
    }

    // Slope from linear regression: lux = slope * pwm + b
    const float denominator = (numPoints * sumX2) - (sumX * sumX);
    float slope = 0.0f;

    if (fabsf(denominator) > 0.0001f) {
      slope = ((numPoints * sumXY) - (sumX * sumY)) / denominator;
    } else {
        slope = 0.0f; // Fallback if points are collinear or too close
    }
    critical_section_enter_blocking(&gStateLock);
    int targetLuminaireId = calibrating_luminaireId;
    critical_section_exit(&gStateLock);
    Serial.print(" Gain of luminaire ");Serial.print(targetLuminaireId);Serial.print(" in luminaire ");
    Serial.print(_luminaireId);Serial.print(": ");Serial.println(slope);

    /*if (slope>7 || slope<=0.0f)
    {
        slope = measuredLux[9]; // Fallback to max lux if slope is unrealistically high, indicating potential measurement error
    }*/
    

   return slope;
}
#include "calibration.h"

void calibrate() {
  
    //Send message that this node wants to calibrate
    if(calibrating_luminaireId!=_luminaireId){
        return; // Only calibrate if this is the intended luminaire
    }
    delay(1000); // Wait for any ongoing operations to settle in core 0 before starting calibration
    
    setPWM(0.0);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.0f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.0f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.0f);
    delay(1000);
    
    setPWM(0.1);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.1f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.1f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.1f);
    if (_luminaireId==0) {
        measuredLux0[0] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[0] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==2) {
        measuredLux2[0] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.2);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.2f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.2f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.2f);
    if (_luminaireId==0) {
        measuredLux0[1] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[1] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[1] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.3);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.3f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.3f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.3f);
    if (_luminaireId==0) {
        measuredLux0[2] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[2] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[2] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.4);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.4f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.4f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.4f);
    if (_luminaireId==0) {
        measuredLux0[3] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[3] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[3] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.5);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.5f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.5f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.5f);
    if (_luminaireId==0) {
        measuredLux0[4] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[4] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[4] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.6);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.6f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.6f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.6f);
    if (_luminaireId==0) {
        measuredLux0[5] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[5] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[5] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.7);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.7f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.7f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.7f);
    if (_luminaireId==0) {
        measuredLux0[6] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[6] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[6] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.8);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.8f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.8f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.8f);
    if (_luminaireId==0) {
        measuredLux0[7] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[7] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[7] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(0.9);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 0.9f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 0.9f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 0.9f);
    if (_luminaireId==0) {
        measuredLux0[8] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[8] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[8] = getavglux(getMovingAverageADC());
    }
    delay(1000);

    setPWM(1.0);
    delay(1000);
    encode_and_send(INTERNAL, _luminaireId, 0, CAN_MSG_CALIBRATION, 1.0f);
    encode_and_send(INTERNAL, _luminaireId, 1, CAN_MSG_CALIBRATION, 1.0f);
    encode_and_send(INTERNAL, _luminaireId, 2, CAN_MSG_CALIBRATION, 1.0f);
    if (_luminaireId==0) {
        measuredLux0[9] = getavglux(getMovingAverageADC());
    }else if (_luminaireId==1) {
        measuredLux1[9] = getavglux(getMovingAverageADC());
    }
    else if (_luminaireId==2) {
        measuredLux2[9] = getavglux(getMovingAverageADC());
    }
    if (calibrating_luminaireId==2)
    {
        updateCalibration(2,1.0f); // Trigger gain computation after the last luminaire finishes calibration
    }
    delay(1000);
    return;
}

void updateCalibration(float src, float pwm) {

    
    if (calibrating==false && _luminaireId==0)
    {
        calibrating=true;
        calibrate(); // Start calibration if not already calibrating and this is the first luminaire
    }
    calibrating = true; // Set calibrating flag to indicate calibration is in progress
    if (pwm == 0.0f){return;}
    
    if(src == 0){
        measuredLux0[int(pwm*10)-1] = getavglux(getMovingAverageADC());
    }else if(src == 1){
        measuredLux1[int(pwm*10)-1] = getavglux(getMovingAverageADC());
    }else if(src == 2){
        measuredLux2[int(pwm*10)-1] = getavglux(getMovingAverageADC());
    }

    if (pwm==1){
        const int srcId = int(src);
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
        controller.setLuminaireGain(srcId, slope);

        calibrating_luminaireId = srcId + 1; // Set to a value that indicates calibration is done for this luminaire
        if(calibrating_luminaireId>=3){
            calibrating_luminaireId=0; // Reset for potential future calibrations
            calibrating = false; // Clear calibrating flag after all luminaires have been calibrated
        }else if(calibrating_luminaireId==_luminaireId){
            calibrate(); // Start calibration for the next luminaire
        }

    }
    return;
}

float calibrateSystem(const float* measuredLux) {

   const int numPoints = 10;
   float sumX = 0.0f;
   float sumY = 0.0f;
   float sumXY = 0.0f;
   float sumX2 = 0.0f;

   for (int i = 0; i < numPoints; ++i) {
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

   if (abs(denominator) > 0.0001f) {
      slope = ((numPoints * sumXY) - (sumX * sumY)) / denominator;
   } else {
      Serial.println("Error: Calibration failed (No light change detected)");
   }

   Serial.println("--- Calibration Complete ---");
   Serial.print("Declive (Lux/PWM): ");
   Serial.println(slope);

   return slope;
}
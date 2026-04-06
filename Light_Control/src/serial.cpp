#include "serial.h"

static bool mapCommandToCanMsgType(const Command& cmd, uint8_t& outMsgType) {
  if (cmd.mainCmd == "u") {
    outMsgType = CAN_MSG_SET_DUTY;
    return true;
  }
  if (cmd.mainCmd == "r") {
    outMsgType = CAN_MSG_SET_ILLUM_REF;
    return true;
  }
  if (cmd.mainCmd == "o") {
    outMsgType = CAN_MSG_SET_OCCUPANCY;
    return true;
  }
  if (cmd.mainCmd == "a") {
    outMsgType = CAN_MSG_SET_ANTI_WINDUP;
    return true;
  }
  if (cmd.mainCmd == "f") {
    outMsgType = CAN_MSG_SET_FEEDBACK;
    return true;
  }
  if (cmd.mainCmd == "O") {
    outMsgType = CAN_MSG_SET_REF_BOUND_HIGH;
    return true;
  }
  if (cmd.mainCmd == "U") {
    outMsgType = CAN_MSG_SET_REF_BOUND_LOW;
    return true;
  }
  if (cmd.mainCmd == "C") {
    outMsgType = CAN_MSG_SET_ENERGY_COST;
    return true;
  }
  if (cmd.mainCmd == "R") {
    outMsgType = CAN_MSG_RESTART;
    return true;
  }
  if (cmd.mainCmd == "g") {
    if (cmd.subCmd == "u") {
      outMsgType = CAN_MSG_GET_DUTY;
      return true;
    }
    if (cmd.subCmd == "r") {
      outMsgType = CAN_MSG_GET_ILLUM_REF;
      return true;
    }
    if (cmd.subCmd == "y") {
      outMsgType = CAN_MSG_GET_LUX;
      return true;
    }
    if (cmd.subCmd == "v") {
      outMsgType = CAN_MSG_GET_LDR_VOLTAGE;
      return true;
    }
    if (cmd.subCmd == "o") {
      outMsgType = CAN_MSG_GET_OCCUPANCY;
      return true;
    }
    if (cmd.subCmd == "a") {
      outMsgType = CAN_MSG_GET_ANTI_WINDUP;
      return true;
    }    if (cmd.subCmd == "O") {
      outMsgType = CAN_MSG_GET_REF_BOUND_HIGH;
      return true;
    }
    if (cmd.subCmd == "U") {
      outMsgType = CAN_MSG_GET_REF_BOUND_LOW;
      return true;
    }
    if (cmd.subCmd == "L") {
      outMsgType = CAN_MSG_GET_CURR_REF_BOUND;
      return true;
    }    if (cmd.subCmd == "f") {
      outMsgType = CAN_MSG_GET_FEEDBACK;
      return true;
    }
    if (cmd.subCmd == "b") {
      if (cmd.charValue == 'y') {
        outMsgType = CAN_MSG_GET_STREAM_BUFFER_Y;
        return true;
      }
      if (cmd.charValue == 'u') {
        outMsgType = CAN_MSG_GET_STREAM_BUFFER_U;
        return true;
      }
      return false;
    }
    if (cmd.subCmd == "t") {
      outMsgType = CAN_MSG_GET_ELAPSED_TIME;
      return true;
    }
    if (cmd.subCmd == "d") {
      outMsgType = CAN_MSG_GET_EXT_ILLUM;
      return true;
    }
    if (cmd.subCmd == "V") {
      outMsgType = CAN_MSG_GET_AVG_VIS_ERROR;
      return true;
    }
    if (cmd.subCmd == "F") {
      outMsgType = CAN_MSG_GET_AVG_FLICKER;
      return true;
    }
    if (cmd.subCmd == "E") {
      outMsgType = CAN_MSG_GET_AVG_ENERGY;
      return true;
    }
    if (cmd.subCmd == "p") {
      outMsgType = CAN_MSG_GET_INST_POWER;
      return true;
    }
  }

  return false;
}

static void resetControllerStateToDefaults() {
  critical_section_enter_blocking(&gStateLock);

  // Reset control inputs to startup defaults (no recalibration).
  gInputs.referenceLux = 20.0f;
  gInputs.occupancyState = 'o';
  gInputs.antiWindupEnabled = true;
  gInputs.feedbackEnabled = true;
  gInputs.manualOverride = false;
  gInputs.pwm[0] = 0.0f;
  gInputs.pwm[1] = 0.0f;
  gInputs.pwm[2] = 0.0f;
  gOutputs.energyCost = 0.0f;
  gInputs.refOccupied = 20.0f;
  gInputs.refLow = 20.0f;
  gInputs.refHigh = 20.0f;

  // Reset outputs/history buffers.
  gOutputs.timestampMs = 0;
  gOutputs.duty = 0.0f;
  gOutputs.luxMeasured = 0.0f;
  gOutputs.ldrVoltage = 0.0f;
  gOutputs.ldrResistance = 0.0f;
  gOutputs.instantPower = 0.0f;
  gOutputs.accumulatedEnergy = 0.0f;
  gOutputs.averageVisibilityError = 0.0f;
  gOutputs.averageFlicker = 0.0f;

  memset((void*)&gHistory, 0, sizeof(gHistory));
  memset((void*)&gPending, 0, sizeof(gPending));

  critical_section_exit(&gStateLock);

  controller.reset();
  setPWM(0.0f);
  reset_flicker_metrics();
}

// Helper function to send GET command responses (via Serial or CAN)
static void sendGetResponse(const Command& cmd, float responseValue) {
  if (cmd.origin == ORIGIN_SERIAL) {
    // Send response via Serial (original behavior)
    Serial.print(cmd.mainCmd);
    Serial.print(" ");
    Serial.print(cmd.luminaireId);
    Serial.print(" ");
    Serial.println(responseValue);
  } else if (cmd.origin == ORIGIN_CAN) {
    // Send response via CAN
    uint8_t msgType = 0;
    if (!mapCommandToCanMsgType(cmd, msgType)) {
      // Cannot map command type - should not happen
      Serial.println("err");
      return;
    }
    encode_and_send(FSERIAL, _luminaireId, cmd.sourceLuminaireId, msgType, responseValue);
  }
}

static void sendGetCharResponse(const Command& cmd, char responseValue) {
  if (cmd.origin == ORIGIN_SERIAL) {
    // Occupancy GET response format: o <i> <val>
    Serial.print("o");
    Serial.print(" ");
    Serial.print(cmd.luminaireId);
    Serial.print(" ");
    Serial.println(responseValue);
  } else if (cmd.origin == ORIGIN_CAN) {
    uint8_t msgType = 0;
    if (!mapCommandToCanMsgType(cmd, msgType)) {
      Serial.println("err");
      return;
    }
    encode_and_send_byte(FSERIAL, _luminaireId, cmd.sourceLuminaireId, msgType, static_cast<uint8_t>(responseValue));
  }
}

static void sendGetElapsedTimeResponse(const Command& cmd, float secondsValue) {
  if (cmd.origin == ORIGIN_SERIAL) {
    // Elapsed time GET response format: t <i> <val>
    Serial.print("t");
    Serial.print(" ");
    Serial.print(cmd.luminaireId);
    Serial.print(" ");
    Serial.println(secondsValue);
  } else if (cmd.origin == ORIGIN_CAN) {
    uint8_t msgType = 0;
    if (!mapCommandToCanMsgType(cmd, msgType)) {
      Serial.println("err");
      return;
    }
    encode_and_send(FSERIAL, _luminaireId, cmd.sourceLuminaireId, msgType, secondsValue);
  }
}

static void sendBufferResponse(char variableId, int luminaireId) {
  uint16_t head;
  uint16_t count;

  critical_section_enter_blocking(&gStateLock);
  head = gHistory.head;
  count = gHistory.count;
  critical_section_exit(&gStateLock);

  const uint16_t n = HISTORY_BUFFER_SAMPLES;
  const uint16_t start = static_cast<uint16_t>((head + n - count) % n);

  Serial.println("-------------------------------------------------------");
  Serial.print("Buffer ");
  Serial.print(variableId);
  Serial.print(" desk ");
  Serial.println(luminaireId);
  Serial.print(" time - value ");

  for (uint16_t k = 0; k < count; k++) {
    const uint16_t idx = static_cast<uint16_t>((start + k) % n);
    Serial.print(gHistory.timestampMs[idx]);
    Serial.print(" - ");
    if (variableId == 'y') {
      Serial.println(gHistory.illuminanceLux[idx]);
    } else {
      Serial.println(gHistory.pwmDuty[idx]);
    }
  }
}

// Helper function to send ACK/ERR responses (via Serial or CAN)
static void sendCommandResponse(const Command& cmd, bool success) {
  if (cmd.origin == ORIGIN_SERIAL) {
    // Send response via Serial
    if (success) {
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  } else if (cmd.origin == ORIGIN_CAN) {
    // For CAN: ACK/ERR is encoded in CAN msgType (no payload needed)
    encode_and_send_status(_luminaireId, cmd.sourceLuminaireId, success);
  }
}

static void printStatus() {
  float duty;
  float referenceLux;
  char occupancyState;
  bool antiWindupEnabled;
  bool feedbackEnabled;
  float pwmValues[3];

  critical_section_enter_blocking(&gStateLock);
  duty = gOutputs.duty;
  referenceLux = gInputs.referenceLux;
  occupancyState = gInputs.occupancyState;
  antiWindupEnabled = gInputs.antiWindupEnabled;
  feedbackEnabled = gInputs.feedbackEnabled;
  for (int i = 0; i < 3; i++) {
    pwmValues[i] = gInputs.pwm[i];
  }
  critical_section_exit(&gStateLock);

  Serial.println("-------------------------------------------------------");
  Serial.println("Status");
  Serial.print("Duty: ");
  Serial.println(duty);
  Serial.print("Reference Lux: ");
  Serial.println(referenceLux);
  Serial.print("Occupancy State: ");
  Serial.println(occupancyState);
  Serial.print("Anti-windup Enabled: ");
  Serial.println(antiWindupEnabled ? "true" : "false");
  Serial.print("Feedback Enabled: ");
  Serial.println(feedbackEnabled ? "true" : "false");

  if (pwmValues[0] != 0.0f || pwmValues[1] != 0.0f || pwmValues[2] != 0.0f) {
    Serial.print("PWM: [");
    for (int i = 0; i < 3; i++) {
      Serial.print(pwmValues[i]);
      if (i < 2) {
        Serial.print(", ");
      }
    }
    Serial.println("]");
  }
}

static void printExternalPwm() {
  float pwmValues[3];

  critical_section_enter_blocking(&gStateLock);
  for (int i = 0; i < 3; i++) {
    pwmValues[i] = gInputs.pwm[i];
  }
  critical_section_exit(&gStateLock);

  Serial.println("-------------------------------------------------------");
  Serial.println("PWM received from other luminaires");
  for (int i = 0; i < 3; i++) {
    if (i == _luminaireId) {
      continue;
    }
    Serial.print("PWM[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(pwmValues[i]);
  }
}

void handleSerial() {
  if (!Serial.available()) {
    return;
  }

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.length() > 0) {
    Command cmd = parseCommand(input);

    if (cmd.mainCmd == "R" && cmd.luminaireId < 0) {
      // Allow plain "R" to target the local luminaire.
      cmd.luminaireId = _luminaireId;
    }

    if (cmd.luminaireId < 0 || cmd.luminaireId >= 3) {
      Serial.println("err");
    } else if (cmd.luminaireId == _luminaireId ) {
      executeCommand(cmd);
    } else {
      // Encode message and forward to destination luminaire via CAN
      uint8_t msgType = 0;
      if (!mapCommandToCanMsgType(cmd, msgType)) {
        Serial.println("err");
        return;
      }
      if (cmd.mainCmd == "o") {
        // Occupancy set carries a character state ('o', 'l', 'h').
        encode_and_send_byte(FSERIAL, _luminaireId, cmd.luminaireId, msgType, static_cast<uint8_t>(cmd.charValue));
      } else if (cmd.mainCmd == "g" && cmd.subCmd == "b") {
        // Buffer selector is a character ('y' or 'u').
        encode_and_send_byte(FSERIAL, _luminaireId, cmd.luminaireId, msgType, static_cast<uint8_t>(cmd.charValue));
      } else if (cmd.mainCmd == "a" || cmd.mainCmd == "f") {
        // Boolean set commands are encoded as a single byte (0/1).
        encode_and_send_byte(FSERIAL, _luminaireId, cmd.luminaireId, msgType, (cmd.value == 1.0f) ? 1 : 0);
      } else {
        encode_and_send(FSERIAL, _luminaireId, cmd.luminaireId, msgType, cmd.value);
      }
      waiting_can = true; // Set flag to indicate we're waiting for a CAN response
    }
  }
}

Command parseCommand(String input) {
  
  Command cmd;
  cmd.mainCmd = "";
  cmd.subCmd = "";
  cmd.luminaireId = -1;
  cmd.value = 0.0f;
  cmd.charValue = '\0';
  cmd.origin = ORIGIN_SERIAL;
  cmd.sourceLuminaireId = _luminaireId;
  
  // Tokenize input
  int tokens[5];
  int tokenCount = 0;
  String token = "";
  
  for (int i = 0; i < input.length() && tokenCount < 5; i++) {
    if (input[i] == ' ') {
      if (token.length() > 0) {
        tokens[tokenCount] = token.length();
        tokenCount++;
        token = "";
      }
    } else {
      token += input[i];
    }
  }
  if (token.length() > 0 && tokenCount < 5) {
    tokens[tokenCount] = token.length();
    tokenCount++;
  }
  
  // Extract tokens from input
  String tokens_str[5];
  int pos = 0;
  for (int i = 0; i < tokenCount; i++) {
    tokens_str[i] = input.substring(pos, pos + tokens[i]);
    pos += tokens[i] + 1;
  }
  
  // Hierarchical decoding
  if (tokenCount >= 1) {
    cmd.mainCmd = tokens_str[0];

    // Restart command supports both "R" and "R <i>".
    if (cmd.mainCmd == "R") {
      if (tokenCount >= 2) {
        cmd.luminaireId = tokens_str[1].toInt();
      }
      return cmd;
    }
    
    // 'g' commands have a subcommand
    if (cmd.mainCmd == "g" && tokenCount >= 2) {
      cmd.subCmd = tokens_str[1];

      // Buffer query format: g b <x> <i>
      if (cmd.subCmd == "b") {
        if (tokenCount >= 3 && tokens_str[2].length() == 1) {
          cmd.charValue = tokens_str[2][0];
        }
        if (tokenCount >= 4) {
          cmd.luminaireId = tokens_str[3].toInt();
        }
      } else {
        if (tokenCount >= 3) {
          cmd.luminaireId = tokens_str[2].toInt();
        }
        if (tokenCount >= 4) {
          cmd.value = tokens_str[3].toFloat();
        }
      }
    }
    // Other commands
    else if (tokenCount >= 2) {
      cmd.luminaireId = tokens_str[1].toInt();
      if (tokenCount >= 3) {
        String valStr = tokens_str[2];
        // Check if it's a single character (for occupancy)
        if (valStr.length() == 1 && !isDigit(valStr[0])) {
          cmd.charValue = valStr[0];
        } else {
          cmd.value = valStr.toFloat();
        }
      }
    }
  }
  
  return cmd;
}

void executeCommand(Command cmd) {

  if (cmd.mainCmd == "status" || cmd.mainCmd == "p") {
    printStatus();
  }

  // GET commands (main command 'g')
  else if (cmd.mainCmd == "g") {
    
    if (cmd.subCmd == "u") {
      // Get duty cycle: g u <i>
      float dutyValue;

      critical_section_enter_blocking(&gStateLock);
      dutyValue = gOutputs.duty;
      critical_section_exit(&gStateLock);
      
      sendGetResponse(cmd, dutyValue);
    }

    else if (cmd.subCmd == "r") {
      // Get reference lux: g r <i>
      float refValue;
      
      critical_section_enter_blocking(&gStateLock);
      refValue = gInputs.referenceLux;
      critical_section_exit(&gStateLock);
      
      sendGetResponse(cmd, refValue);
    }

    else if (cmd.subCmd == "y") {
      // Get measured lux: g y <i>
      float lux;
      
      critical_section_enter_blocking(&gStateLock);
      lux = gOutputs.luxMeasured;
      critical_section_exit(&gStateLock);
      
      sendGetResponse(cmd, lux);
    }

    else if (cmd.subCmd == "v") {
      // Get voltage at LDR: g v <i>
      float v_out;
      
      critical_section_enter_blocking(&gStateLock);
      v_out = gOutputs.ldrVoltage;
      critical_section_exit(&gStateLock);
      
      sendGetResponse(cmd, v_out);
    }

    else if (cmd.subCmd == "o") {
      // Get occupancy state: g o <i>
      char occupancyState;

      critical_section_enter_blocking(&gStateLock);
      occupancyState = gInputs.occupancyState;
      critical_section_exit(&gStateLock);

      sendGetCharResponse(cmd, occupancyState);
    }

    else if (cmd.subCmd == "a") {
      // Get anti-windup state: g a <i>
      bool antiWindupEnabled;
      critical_section_enter_blocking(&gStateLock);
      antiWindupEnabled = gInputs.antiWindupEnabled;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, antiWindupEnabled ? 1.0f : 0.0f);
    }

    else if (cmd.subCmd == "f") {
      // Get feedback control state: g f <i>
      bool feedbackEnabled;
      critical_section_enter_blocking(&gStateLock);
      feedbackEnabled = gInputs.feedbackEnabled;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, feedbackEnabled ? 1.0f : 0.0f);
    }

    /*else if (cmd.subCmd == "w") {
      // Get setpoint weighting: g w <i>
      float beta = controller.getWeight(PID::BETA);
      sendGetResponse(cmd, beta);
    }*/

    else if (cmd.subCmd == "d") {
      // Get external luminance: g d <i>
      float externalLuminance = controller.getExternalLuminance();
      sendGetResponse(cmd, externalLuminance);
    }

    else if (cmd.subCmd == "V") {
      // Get average visibility error since restart: g V <i>
      float averageVisibilityError;
      critical_section_enter_blocking(&gStateLock);
      averageVisibilityError = gOutputs.averageVisibilityError;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, averageVisibilityError);
    }

    else if (cmd.subCmd == "F") {
      // Get average flicker error since restart: g F <i>
      float averageFlicker;
      critical_section_enter_blocking(&gStateLock);
      averageFlicker = gOutputs.averageFlicker;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, averageFlicker);
    }

    else if (cmd.subCmd == "C") {
      // Get current energy cost: g C <i>
      float energyCost;
      critical_section_enter_blocking(&gStateLock);
      energyCost = gOutputs.energyCost;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, energyCost);
    }

    else if (cmd.subCmd == "E") {
      // Get accumulated energy since restart: g E <i>
      float accumulatedEnergy;
      critical_section_enter_blocking(&gStateLock);
      accumulatedEnergy = gOutputs.accumulatedEnergy;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, accumulatedEnergy);
    }

    else if (cmd.subCmd == "b") {
      // Get last minute buffer: g b <x> <i>, x in {'y','u'}.
      if (cmd.charValue == 'y' || cmd.charValue == 'u') {
        sendBufferResponse(cmd.charValue, cmd.luminaireId);

        // For remote invocation over CAN, only ACK/ERR is returned.
        if (cmd.origin == ORIGIN_CAN) {
          sendCommandResponse(cmd, true);
        }
      } else {
        sendCommandResponse(cmd, false);
      }
    }

    else if (cmd.subCmd == "t") {
      // Get elapsed time since restart in seconds: g t <i>
      float elapsedSeconds = static_cast<float>(millis()) / 1000.0f;
      sendGetElapsedTimeResponse(cmd, elapsedSeconds);
    }

    else if (cmd.subCmd == "O") {
      // Get reference illuminance for HIGH state: g O <i>
      float refHigh;
      critical_section_enter_blocking(&gStateLock);
      refHigh = gInputs.refHigh;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, refHigh);
    }

    else if (cmd.subCmd == "U") {
      // Get reference illuminance for LOW state: g U <i>
      float refLow;
      critical_section_enter_blocking(&gStateLock);
      refLow = gInputs.refLow;
      critical_section_exit(&gStateLock);
      sendGetResponse(cmd, refLow);
    }

    else if (cmd.subCmd == "L") {
      // Get current lower bound based on occupancy state: g L <i>
      float currentLowerBound;
      critical_section_enter_blocking(&gStateLock);
      char occupState = gInputs.occupancyState;
      float refOcc = gInputs.refOccupied;
      float refL = gInputs.refLow;
      float refH = gInputs.refHigh;
      critical_section_exit(&gStateLock);
      
      // Select appropriate bound based on current occupancy state
      if (occupState == 'h') {
        currentLowerBound = refH;
      } else if (occupState == 'l') {
        currentLowerBound = refL;
      } else {
        currentLowerBound = refOcc;  // 'o' state
      }
      sendGetResponse(cmd, currentLowerBound);
    }

    else if (cmd.subCmd == "p") {
      // Get instantaneous power consumption: g p <i>
      float pwr = getInstantPower();
      sendGetResponse(cmd, pwr);
    }
  }
  else if (cmd.mainCmd == "C") {
    // Set current energy cost: C <i> <val>
    if (cmd.luminaireId < 3 && cmd.value >= 0.0f) {
      critical_section_enter_blocking(&gStateLock);
      gOutputs.energyCost = cmd.value;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  // SET commands
  else if (cmd.mainCmd == "u") {
    // Set duty cycle: u <i> <val>
    // Activates manual override mode (independent of feedback state).
    if (cmd.value >= 0.0f && cmd.value <= 1.0f) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasDuty = true;
      gPending.newDuty = cmd.value;
      gPending.hasManualOverride = true;
      gPending.newManualOverride = true;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "r") {
    // Set reference lux: r <i> <val>
    if (cmd.luminaireId < 3) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasReferenceLux = true;
      gPending.newReferenceLux = cmd.value;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "o") {
    // Set occupancy state: o <i> <val>
    if (cmd.luminaireId < 3 && (cmd.charValue == 'o' || cmd.charValue == 'l' || cmd.charValue == 'h')) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasOccupancyState = true;
      gPending.newOccupancyState = cmd.charValue;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "a") {
    // Set anti-windup: a <i> <val>
    if (cmd.luminaireId < 3 && (cmd.value == 0.0f || cmd.value == 1.0f)) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasAntiWindupEnabled = true;
      gPending.newAntiWindupEnabled = (cmd.value == 1.0f);
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "f") {
    // Set feedback control: f <i> <val>
    // Feedback is independent of manual override.
    if (cmd.luminaireId < 3 && (cmd.value == 0.0f || cmd.value == 1.0f)) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasFeedbackEnabled = true;
      gPending.newFeedbackEnabled = (cmd.value == 1.0f);
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "m") {
    // Set manual override: m <i> <val>
    // m <i> 0 = disable override, m <i> 1 = enable override
    if (cmd.luminaireId < 3 && (cmd.value == 0.0f || cmd.value == 1.0f)) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasManualOverride = true;
      gPending.newManualOverride = (cmd.value == 1.0f);
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "O") {
    // Set reference illuminance for HIGH state: O <i> <val>
    if (cmd.luminaireId < 3 && cmd.value >= 0.0f) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasRefHigh = true;
      gPending.newRefHigh = cmd.value;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "U") {
    // Set reference illuminance for LOW state: U <i> <val>
    if (cmd.luminaireId < 3 && cmd.value >= 0.0f) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasRefLow = true;
      gPending.newRefLow = cmd.value;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  else if (cmd.mainCmd == "R") {
    // Restart logical state: reset values to defaults (without recalibration).
    if (cmd.luminaireId < 3) {
      resetControllerStateToDefaults();
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }
  /*else if (cmd.mainCmd == "w") {
    // Set setpoint weighting: w <i> <val>
    if (cmd.luminaireId == 0) {
      critical_section_enter_blocking(&gStateLock);
      gPending.hasBeta = true;
      gPending.newBeta = cmd.value;
      critical_section_exit(&gStateLock);
      sendCommandResponse(cmd, true);
    } else {
      sendCommandResponse(cmd, false);
    }
  }*/
  else {
    sendCommandResponse(cmd, false);
  }
}



void print_to_serial() {
#if !STREAM_TELEMETRY_ENABLED
  return;
#else
  uint32_t timestamp;
  float luxMeasured;
  float duty;

  critical_section_enter_blocking(&gStateLock);
  timestamp = gOutputs.timestampMs;
  luxMeasured = gOutputs.luxMeasured;
  duty = gOutputs.duty;
  critical_section_exit(&gStateLock);

  // Compact stream format for realtime plotting.
  // time,luminaire_id,lux_meas,duty_cycle
  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(_luminaireId);
  Serial.print(",");
  Serial.print(luxMeasured);
  Serial.print(",");
  Serial.println(duty);
#endif
}
#include <Arduino.h>
/*
   Distributed Real-Time Control Systems Project
   Luminaire Local Controller - Complete Version
   Platform: Raspberry Pi Pico (Arduino IDE)
   Sampling: 100 Hz
*/

#define PWM_PIN 9 // PWM output to LED
#define LDR_PIN 26  // ADC input from LDR voltage divider


#define TS 0.01 // Control period in seconds (10 ms)
#define PWM_MAX 255 // Maximum PWM value for 8-bit resolution

#define R_FIXED 10000.0  // 10k resistor in divider





int luminaireId = 0; // Only one luminaire in this implementation

float m = -0.8; // Slope of log-log curve (assumed)
float b = 6.10; // Intercept of log-log curve (from calibration)


float visibility_error = 0;
float visibility_error_sum = 0;
int visibility_error_counter = 0;

int samplecounter=0;



float referenceLux = 20.0;   // reference lux level
float duty = 0.0; //duty cycle (0.0 to 1.0)

char occupancyState = 'o';  // 'o' - off, 'l' - low, 'h' - high
bool antiWindupEnabled = true;  // anti-windup bit
bool feedbackEnabled = true;  
bool luminancecontrolEnabled = true;
float beta = 1.0; // setpoint weighting parameter for proportional term


struct StreamData {
  unsigned long timestamp;
  int luminaireId;
  float lux_ref;
  float lux_meas;
  float ldrVoltage;
  float ldrResistance;
  float dutyCycle;
  bool antiWindupState;
  float beta; // setpoint weighting parameter
};

StreamData serial_data;


float luxSum = 0.0;
int luxsumADC = 0;
int sampleCount = 0;
int sampleCountADC = 0;

class PID {
private:
    float Kp, Ki, Kd;       // ganhos PID
    float beta, gamma;       // setpoint weighting
    float Kff;               // ganho feedforward
    float Ts;                // período de amostragem

    float integrator;
    float previousMeasurement;
    float previousReference;

    float outputMin, outputMax;

    bool useFeedforward;     // se true usa FF, se false usa feedback (PID completo)
    bool antiWindup;         // habilita anti-windup

public:
    PID(float kp, float ki, float kd,
        float b, float g,
        float ts,
        float kff,
        bool useFF,
        bool antiWindupEnabled)
    {
        Kp = kp; Ki = ki; Kd = kd;
        beta = b; gamma = g;
        Kff = kff;
        Ts = ts;

        integrator = 0.0;
        previousMeasurement = 0.0;
        previousReference = 0.0;

        outputMin = 0.0;
        outputMax = 1.0;

        useFeedforward = useFF;
        antiWindup = antiWindupEnabled;
    }

    void reset() {
        integrator = 0.0;
        previousMeasurement = 0.0;
        previousReference = 0.0;
    }

    float compute(float reference, float measurement) {

        if (useFeedforward) {
            // modo feedforward puro
            float output = Kff * reference;
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

        if (antiWindup) {
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

    void setOutputLimits(float minVal, float maxVal) {
        outputMin = minVal;
        outputMax = maxVal;
    }

    void setsetpointWeighting(float b, float g) {
        beta = b;
        gamma = g;
    }

    void setModeFeedforward(bool enable) {
        useFeedforward = enable;
    }

    void setAntiWindup(bool enable) {
        antiWindup = enable;
    }
};

bool antiWindup = true;
bool useFeedforward = false;

// modo feedforward
PID controller(0.02, 0.1, 0.0,  // Kp, Ki, Kd
               1.0, 0.0,       // beta, gamma
               TS,           // Ts
               1.0,            // Kff
               useFeedforward,           // use feedforward
               antiWindup);

// mudar para feedback


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
  serial_data.ldrVoltage = v_out;
  float r_ldr = getLDRresistance(v_out);
  serial_data.ldrResistance = r_ldr;
  float log10_ldr = log10(r_ldr);
  float avglux = pow(10, (log10_ldr - b) / m);
  serial_data.lux_meas = avglux;
  return avglux;
}


void setPWM(float dutyCycle) {

  if (dutyCycle > 1.0) dutyCycle = 1.0;
  if (dutyCycle < 0.0) dutyCycle = 0.0;

  analogWrite(PWM_PIN, dutyCycle * PWM_MAX);
}

// =====================
// COMMAND PARSING & EXECUTION
// =====================

struct Command {
  String mainCmd;      // First token: 'u', 'r', 'o', 'a', 'f', 's', 'S', 'g'
  String subCmd;       // Second token (for get commands): 'u', 'r', 'y', 'v', 'o', 'a', 'f', 'd', 'p', 't', 'b'
  int luminaireId;     // Luminaire index <i>
  float value;         // Value for set commands
  char charValue;      // Character value for occupancy states
};

Command parseCommand(String input) {
  
  Command cmd;
  cmd.mainCmd = "";
  cmd.subCmd = "";
  cmd.luminaireId = -1;
  cmd.value = 0.0;
  cmd.charValue = '\0';
  
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
    
    // 'g' commands have a subcommand
    if (cmd.mainCmd == "g" && tokenCount >= 2) {
      cmd.subCmd = tokens_str[1];
      
      if (tokenCount >= 3) {
        cmd.luminaireId = tokens_str[2].toInt();
      }
      if (tokenCount >= 4) {
        cmd.value = tokens_str[3].toFloat();
      }
    }
    // 's' (stream start) and 'S' (stream stop)
    else if ((cmd.mainCmd == "s" || cmd.mainCmd == "S") && tokenCount >= 2) {
      cmd.subCmd = tokens_str[1];
      if (tokenCount >= 3) {
        cmd.luminaireId = tokens_str[2].toInt();
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
  
  // GET commands (main command 'g')
  if (cmd.mainCmd == "g") {
    
    if (cmd.subCmd == "u") {
      // Get duty cycle: g u <i>
      if (cmd.luminaireId == 0) {
        Serial.print("u ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(duty);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "r") {
      // Get reference lux: g r <i>
      if (cmd.luminaireId == 0) {
        Serial.print("r ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(referenceLux);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "y") {
      // Get measured lux: g y <i>
      if (cmd.luminaireId == 0) {
        float lux = getMovingAverageADC();
        Serial.print("y ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(lux);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "v") {
      // Get voltage at LDR: g v <i>
      if (cmd.luminaireId == 0) {
        int raw = analogRead(LDR_PIN);
        float v_out = (raw * 3.3) / 4095.0;
        Serial.print("v ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(v_out);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "o") {
      // Get occupancy state: g o <i>
      if (cmd.luminaireId == 0) {
        Serial.print("o ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(occupancyState);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "a") {
      // Get anti-windup state: g a <i>
      if (cmd.luminaireId == 0) {
        Serial.print("a ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(antiWindupEnabled);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "f") {
      // Get feedback control state: g f <i>
      if (cmd.luminaireId == 0) {
        Serial.print("f ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(feedbackEnabled);
      } else {
        Serial.println("err");
      }
    }
    else if (cmd.subCmd == "w") {
      // Get setpoint weighting: g w <i>
      if (cmd.luminaireId == 0) {
        Serial.print("w ");
        Serial.print(cmd.luminaireId);
        Serial.print(" ");
        Serial.println(beta);
      } else {
        Serial.println("err");
      }
    }
    else {
      Serial.println("err");
    }
  }
  // SET commands
  else if (cmd.mainCmd == "u") {
    // Set duty cycle: u <i> <val>
    if (cmd.luminaireId == 0 && cmd.value >= 0.0 && cmd.value <= 1.0) {
      duty = cmd.value;
      feedbackEnabled = false;
      setPWM(duty);
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else if (cmd.mainCmd == "r") {
    // Set reference lux: r <i> <val>
    if (cmd.luminaireId == 0) {
      referenceLux = cmd.value;
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else if (cmd.mainCmd == "o") {
    // Set occupancy state: o <i> <val>
    if (cmd.luminaireId == 0 && (cmd.charValue == 'o' || cmd.charValue == 'l' || cmd.charValue == 'h')) {
      occupancyState = cmd.charValue;
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else if (cmd.mainCmd == "a") {
    // Set anti-windup: a <i> <val>
    if (cmd.luminaireId == 0 && (cmd.value == 0.0 || cmd.value == 1.0)) {
      antiWindupEnabled = (cmd.value == 1.0);
      controller.setAntiWindup(antiWindupEnabled);
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else if (cmd.mainCmd == "f") {
    // Set feedback control: f <i> <val>
    if (cmd.luminaireId == 0 && (cmd.value == 0.0 || cmd.value == 1.0)) {
      feedbackEnabled = (cmd.value == 1.0);
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else if (cmd.mainCmd == "w") {
    // Set setpoint weighting: w <i> <val>
    if (cmd.luminaireId == 0) {
      beta = cmd.value;
      controller.setsetpointWeighting(cmd.value, 0.0); // Only beta is used in this implementation
      Serial.println("ack");
    } else {
      Serial.println("err");
    }
  }
  else {
    Serial.println("err");
  }
}

void handleSerial() {

  if (Serial.available()) {

    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      Command cmd = parseCommand(input);
      executeCommand(cmd);
    }
  }
}

void print_to_serial() {
  //time,luminaire_ID,lux_ref,lux_meas,LDRvoltage,LDRresistance,duty_cycle,windup_state,setpoint-weight
  Serial.print(serial_data.timestamp);
  Serial.print(",");
  Serial.print(serial_data.luminaireId);
  Serial.print(",");
  Serial.print(serial_data.lux_ref);
  Serial.print(",");
  Serial.print(serial_data.lux_meas);
  Serial.print(",");
  Serial.print(serial_data.ldrVoltage);
  Serial.print(",");
  Serial.print(serial_data.ldrResistance);
  Serial.print(",");
  Serial.print(serial_data.dutyCycle);
  Serial.print(",");
  Serial.print(serial_data.antiWindupState);
  Serial.print(",");
  Serial.println(serial_data.beta);
}


unsigned long lastTime = 0;

void setup() {


  Serial.begin(115200);

  analogReadResolution(12);

  pinMode(PWM_PIN, OUTPUT);

  analogWrite(PWM_PIN, 0);


  Serial.println("System ready");
}


void loop() {

  unsigned long now = millis(); // Convert to milliseconds with microsecond precision
  if (now - lastTime >= 10) { // 10ms control period
     
    lastTime = now;
    serial_data.timestamp = now;
    serial_data.luminaireId = luminaireId;
    serial_data.lux_ref = referenceLux;

    // Compute average of all samples collected in the last 10ms    
    float lux = getavglux(getMovingAverageADC());
    
    visibility_error_sum+=(referenceLux-lux);
    visibility_error_counter++;
    
    samplecounter += duty*0.01;


    if(visibility_error_counter==1000){
      visibility_error = visibility_error_sum / 1000.0;
      visibility_error_sum = 0;
      visibility_error_counter = 0;
      Serial.print("visibility_error ");
      Serial.println(visibility_error);
      Serial.print("Power estimate ");
      Serial.println(samplecounter);
    }

    serial_data.antiWindupState = antiWindupEnabled;
    serial_data.beta = beta;

    if (feedbackEnabled && luminancecontrolEnabled){
      duty = controller.compute(referenceLux, lux);
      serial_data.dutyCycle = duty;
    }
    setPWM(duty);
    print_to_serial();
  }
  else {
    // Collect samples between control periods
    addSampleToBufferADC();
  }
  handleSerial();
}
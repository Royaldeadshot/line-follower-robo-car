/* ============================================================
   LINE FOLLOWER ROBOT  ESP32 + PID Control
   Sensor: SmartElex 5-Channel Analog IR Tracker
   Driver: L298N Dual Motor Driver
   Motors: 2x 300 RPM BO Motor
   ============================================================ */

// ───────────── SENSOR PINS (analog input only) ─────────────
const int IR1 = 34; //Leftmost
const int IR2 = 35;
const int IR3 = 32; //Center
const int IR4 = 33;
const int IR4 = 25; //Rightmost

// ───────────── MOTOR DRIVER PINS ─────────────
//Left motor
const int IN1 = 26;
const int IN2 = 27;
const int ENA_PIN = 14;
const int ENA_CH = 0;

//RIght motor
const int IN3 = 22;
const int IN4 = 23;
const int ENB_PIN = 13;
const int ENB_CH = 0;

const int PWM_FREQ = 1000;
const int PWM_RES = 8;    //8 bit -> 0-255

// ───────────── SPEED SETTINGS ─────────────
const int BASE_SPEED = 160;     // normal cruising speed (0-255)
const int MAX_SPEED  = 230;     //upper PWM cap
const int MIN_SPEED  = 70;      //lowest PWM that still moves the motor

// ───────────── PID CONSTANTS — TUNE THESE ─────────────
// Start with kp only, then add kd, then a litlle ki if needed.
float kp = 18.0;
float ki = 0.0;
float kd = 10.0;

// ───────────── SENSOR CALIBRATION ─────────────
// Fill theese after running the calibration routine below.
int minVal[5] = {0, 0, 0, 0, 0};    // raw reading on white surface
int maxVal[5] = {4095, 4095, 4095, 4095, 4095}; // raw reading on black line

// ───────────── PID STATE ─────────────
float lasterror = 0;
float integrl   = 0;

// Set true once to run calibration on boot
const bool RUN_CALIBRATION = false;

void setup() {
  Serial.begin(115200);

  pinmode(IR1, INPUT);
  pinmode(IR2, INPUT);
  pinmode(IR3, INPUT);
  pinmode(IR4, INPUT);
  pinmode(IR5, INPUT);

  pinmode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinmode(IN3, OUTPUT);
  pinmode(IN4, OUTPUT);

  ledcSetup(ENA_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA_PIN, ENA_CH);
  ledcSetup(ENB_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENB_PIN, ENB_CH);

  if (RUN_CALIBRATION) {
    calibrateSensors();
  }

  delay(1000); // pause before robot starts moving
}

void loop() {
  int raw[5] = {
    analogRead(IR1),
    analogRead(IR2),
    analogRead(IR3),
    analogRead(IR4),
    analogRead(IR5)
  };

  // Normalize each reading to 0-1000 using calibration values
  int norm[5];
  for (int i = 0; i < 5; i++) {
    norm[i] = map(raw[i], minVal[i], maxVal[i], 0, 1000);
    norm[i] = constrain(norm[i], 0, 1000);
  }

  // Check if line is lost (all sensors read low / white)
  long total = norm[0] + norm[1] + norm[2] + norm[3] + norm[4];

  if (total < 250) {
    // Line lost — stop or implement search routine
    handleLineLost();
    return;
  }

  // Weighted position: -2000 (far left) to +2000 (far right), 0 = centered
  float weightedSum = (norm[0] * -2000.0) + (norm[1] * -1000.0) +
                       (norm[2] * 0.0)     + (norm[3] * 1000.0)  +
                       (norm[4] * 2000.0);
  float position = weightedSum / total;

  // ───────────── PID CALCULATION ─────────────
  float error = position; // target is 0 (centered)
  integral += error;
  integral = constrain(integral, -3000, 3000); // prevent windup
  float derivative = error - lastError;

  float correction = (Kp * error / 1000.0) + (Ki * integral / 1000.0) + (Kd * derivative / 1000.0);
  lastError = error;

  // Apply correction to base speed
  int leftSpeed  = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;

  leftSpeed  = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  driveMotor(leftSpeed, rightSpeed);

  // Debug output — open Serial Monitor at 115200 baud
  Serial.printf("pos:%.0f  err:%.0f  corr:%.1f  L:%d  R:%d\n",
                position, error, correction, leftSpeed, rightSpeed);

  delay(5); // small loop delay for stability
}

// ───────────── MOTOR CONTROL ─────────────
void driveMotor(int leftSpeed, int rightSpeed) {
  // LEFT MOTOR
  if (leftSpeed >= 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(ENA_CH, applyMinSpeed(leftSpeed));
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(ENA_CH, applyMinSpeed(-leftSpeed));
  }

  // RIGHT MOTOR
  if (rightSpeed >= 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENB_CH, applyMinSpeed(rightSpeed));
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(ENB_CH, applyMinSpeed(-rightSpeed));
  }
}

// Ensures PWM never drops so low the motor stalls, unless speed is truly 0
int applyMinSpeed(int speed) {
  if (speed == 0) return 0;
  if (speed < MIN_SPEED) return MIN_SPEED;
  return speed;
}

void stopMotors() {
  ledcWrite(ENA_CH, 0);
  ledcWrite(ENB_CH, 0);
}

// ───────────── LINE LOST HANDLER ─────────────
void handleLineLost() {
  // Simple version: stop.
  // Upgrade idea: turn in the direction of lastError to search for the line again.
  stopMotors();
  Serial.println("Line lost!");
}

// ───────────── CALIBRATION ROUTINE ─────────────
// Run this once with RUN_CALIBRATION = true.
// Slowly slide the sensor across the line by hand for ~3 seconds
// while it records min/max values for each channel.
void calibrateSensors() {
  Serial.println("Calibrating... slide sensor across line now.");

  int pins[5] = {IR1, IR2, IR3, IR4, IR5};

  for (int i = 0; i < 5; i++) {
    minVal[i] = 4095;
    maxVal[i] = 0;
  }

  unsigned long startTime = millis();
  while (millis() - startTime < 4000) {
    for (int i = 0; i < 5; i++) {
      int val = analogRead(pins[i]);
      if (val < minVal[i]) minVal[i] = val;
      if (val > maxVal[i]) maxVal[i] = val;
    }
    delay(10);
  }

  Serial.println("Calibration done. Copy these values into your code:");
  for (int i = 0; i < 5; i++) {
    Serial.printf("Sensor %d -> min: %d  max: %d\n", i + 1, minVal[i], maxVal[i]);
  }
  Serial.println("Update minVal[] and maxVal[] arrays, then set RUN_CALIBRATION to false.");
  delay(5000);
}

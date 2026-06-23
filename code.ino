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
  pinmode(IN3, OUTPUT);
  pinmode(IN4, OUTPUT);

}
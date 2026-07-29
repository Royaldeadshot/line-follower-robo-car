# Line Follower Robot

A line following robot built with an ESP32, a 5 channel analog IR sensor, and PID control for smooth tracking.

## Components

| Part | Spec |
|---|---|
| Microcontroller | ESP32 NodeMCU (30 pin, CP2102) |
| IR Sensor | SmartElex 5 Channel Analog Tracker |
| Motor Driver | L298N Dual H Bridge (2A) |
| Motors | 2x 300 RPM BO Motor (dual shaft) |
| Battery | 2x 18650 Li ion (7.4V) |
| Chassis | 3D printed, JSumo style skeleton design |

## How It Works

The 5 channel sensor reads the line position as an analog value per channel. The ESP32 converts these into a single weighted position (-2000 to +2000), then runs a PID loop to correct motor speed and keep the robot centered on the line. And PID control ensures that the robo doesnt overshoot and wobble badly 

## Wiring
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/8336bc57-21ed-4d26-b352-cb1f89331a2f" />

| Sensor Pin | ESP32 GPIO |
|---|---|
| IR1 (left) | 34 |
| IR2 | 35 |
| IR3 (center) | 32 |
| IR4 | 33 |
| IR5 (right) | 25 |

| ESP32 GPIO | L298N Pin |
|---|---|
| 26 | IN1 |
| 27 | IN2 |
| 14 | ENA |
| 22 | IN3 |
| 23 | IN4 |
| 13 | ENB |

Power: Battery → L298N (12V in) → 5V out → ESP32 VIN. All GNDs common.

## Setup

1. Open code.ino in arduino IDE
2. Install ESP32 board support (Espressif)
3. Set RUN_CALIBRATION = true, upload, open Serial Monitor (115200 baud)
4. Slide the sensor across the line for 4 seconds, copy the printed min/max values into minVal[] and maxVal[]
5. Set RUN_CALIBRATION = false, re upload
6. Place robot on the track and power on

## PID Tuning

Start with tuning Kp only (Ki = 0, Kd = 0):

1. Increase Kp until the robot follows the line but wobbles
2. Add Kd to smooth out the wobble
3. Add a small Ki only if the robot drifts consistently to one side

Current values in code: `Kp = 18.0`, `Ki = 0.0`, `Kd = 10.0`


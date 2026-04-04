#include <Arduino.h>
#include <DriveTrain.h>
#include <IMU.h>
#include <Bluetooth.h>
#include <Sensors.h>

void initmotors() {
  pinMode(D1IN1, OUTPUT);
  pinMode(D1IN2, OUTPUT);
  pinMode(D1ENA, OUTPUT);
  pinMode(D1IN3, OUTPUT);
  pinMode(D1IN4, OUTPUT);
  pinMode(D1ENB, OUTPUT);
}

void drive(int leftspeed, int rightspeed)
{
    leftspeed = -leftspeed;
    if (rightspeed > 0)
    {
        digitalWrite(D1IN1, HIGH);
        digitalWrite(D1IN2, LOW);
    }

    else
    {
        digitalWrite(D1IN1, LOW);
        digitalWrite(D1IN2, HIGH);
        rightspeed = -rightspeed;
    }

    analogWrite(D1ENA, constrain(rightspeed, 0, 255));

    if (leftspeed > 0)
    {
        digitalWrite(D1IN3, HIGH);
        digitalWrite(D1IN4, LOW);
    }

    else
    {
        digitalWrite(D1IN3, LOW);
        digitalWrite(D1IN4, HIGH);
        leftspeed = -leftspeed;
    }

    analogWrite(D1ENB, constrain(leftspeed, 0, 255));
}

void motorstop()
{
    analogWrite(D1ENA, 0);
    analogWrite(D1ENB, 0);
}

void turndegrees(float targetDegrees) {
  float TURN_BRAKE_OFFSET;
  targetDegrees = -targetDegrees;
  float startingHeading = heading;
  
  // Proportional Tuning Variables
  float kp = 1.5;
  int minSpeed = 55;
  int maxSpeed = 100;
  
  int dir = targetDegrees > 0 ? 1 : -1;
  
  // Your custom asymmetric offsets
  if (dir < 0){
    TURN_BRAKE_OFFSET = 0;  // degrees to stop early, tune empirically
  } else {
    TURN_BRAKE_OFFSET = 5;
  }

  // Calculate exactly how far the robot needs to travel to trigger the stop
  float targetMagnitude = abs(targetDegrees) + TURN_BRAKE_OFFSET;

  lastIMUTime = micros();
  unsigned long startTime = micros();

  while (true) {
    if (micros() - startTime > TURN_TIMEOUT_US) break;

    IMUData imu = readIMU();
    unsigned long now = micros();
    float dt = (now - lastIMUTime) / 1000000.0f;
    lastIMUTime = now;

    if (abs(imu.gz) > GYRO_HPF) {
      heading -= imu.gz * dt;
    }

    float relativeTurn = heading - startingHeading;
    float currentMagnitude = abs(relativeTurn);
    
    // Calculate degrees remaining until you hit your offset target
    float error = targetMagnitude - currentMagnitude;

    // 1. Stop Condition (Your logic: if we hit or pass the offset target, break)
    if (error <= 0) {
        break; 
    }

    // 2. Proportional Speed Calculation
    int turnPower = error * kp;

    // 3. Constrain the power bounds
    if (turnPower > maxSpeed) turnPower = maxSpeed;
    if (turnPower < minSpeed) turnPower = minSpeed;

    // 4. Update the motor speeds continuously
    drive(-dir * turnPower, dir * turnPower);
  }

  motorstop();
  delay(100);
}


// void turndegrees(float targetDegrees) {
//   float TURN_BRAKE_OFFSET;
//   targetDegrees = -targetDegrees;
//   float startingHeading = heading;
//   float kp = 2.5;
//   int minSpeed = 30;
//   int maxSpeed = 80;
//   int dir = targetDegrees > 0 ? 1 : -1;
//   if (dir < 0){
//     TURN_BRAKE_OFFSET = -2.5;  // degrees to stop early, tune empirically
//   }
//   else {
//     TURN_BRAKE_OFFSET = 0;
//   }

//   drive(-dir * TURN_SPEED, dir * TURN_SPEED);

//   lastIMUTime = micros();
//   unsigned long startTime = micros();

//   while (true) {
//     if (micros() - startTime > TURN_TIMEOUT_US) break;

//     IMUData imu = readIMU();
//     unsigned long now = micros();
//     float dt = (now - lastIMUTime) / 1000000.0f;
//     lastIMUTime = now;

//     if (abs(imu.gz) > GYRO_HPF) {
//       heading -= imu.gz * dt;
//     }

//     float relativeTurn = heading - startingHeading;
//     if (abs(relativeTurn) >= abs(targetDegrees) + TURN_BRAKE_OFFSET) break;
//   }

//   motorstop();
//   delay(50);
// }

void driveforward(int durationMS, int speed) {
  float targetHeading = heading;
  int baseSpeed = speed;
  float kp = 5.0;
  unsigned long startTime = millis();
  lastIMUTime = micros();

  while (millis() - startTime < (unsigned long)durationMS) {
    IMUData imu = readIMU();
    unsigned long now = micros();
    float dt = (now - lastIMUTime) / 1000000.0f;
    lastIMUTime = now;

    if (abs(imu.gz) > GYRO_HPF) {
      heading -= imu.gz * dt;  // note: minus to match turndegrees
    }

    float error = targetHeading - heading;
    int correction = (int)(error * kp);
    correction = constrain(correction, -30, 30);
    int leftSpeed = baseSpeed + correction;
    int rightSpeed = baseSpeed - correction;
    drive(leftSpeed, rightSpeed);
  }

  motorstop();
}

void driveforwardUT(int speed) {
  float baseSpeed = speed;
  float kp = 5.0;

  IMUData imu = readIMU();
  unsigned long now = micros();
  float dt = (now - lastIMUTime) / 1000000.0f;
  lastIMUTime = now;

  if (abs(imu.gz) > GYRO_HPF) {
    heading -= imu.gz * dt;
  }

  float error = targetHeading - heading;  // targetHeading needs to be accessible
  int correction = (int)(error * kp);
  correction = constrain(correction, -30, 30);
  drive(baseSpeed + correction, baseSpeed - correction);
}
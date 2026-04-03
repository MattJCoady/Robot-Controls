#include <Arduino.h>
#include <DriveTrain.h>
#include <IMU.h>
#include <Bluetooth.h>

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
    digitalWrite(D1IN1, LOW);
    digitalWrite(D1IN2, LOW);
    digitalWrite(D1IN3, LOW);
    digitalWrite(D1IN4, LOW);

    analogWrite(D1ENA, 0);
    analogWrite(D1ENB, 0);
}



void turndegrees(float targetDegrees) {
  targetDegrees = -targetDegrees;
  float startingHeading = heading;
  int dir = targetDegrees > 0 ? 1 : -1;

  drive(dir * TURN_SPEED, dir * TURN_SPEED);

  lastIMUTime = micros();
  unsigned long startTime = micros();
  int printCounter = 0; 

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
    if (printCounter++ % 50 == 0) {
      char numBuf[10];
      char buf[20];
      dtostrf(relativeTurn, 6, 2, numBuf);
      snprintf(buf, sizeof(buf), "RT:%s", numBuf);
      sendBluetooth(buf);
    }
    if (abs(relativeTurn) >= abs(targetDegrees) + TURN_BRAKE_OFFSET) break;
  }

  motorstop();
  delay(50);
}

void driveforward(int durationMS,int speed)
{ unsigned long startTime = millis();
  drive(speed,speed);
  while (millis() - startTime < (unsigned long)durationMS) {

  }
  motorstop();
}
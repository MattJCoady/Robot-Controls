#pragma once
#include <Arduino.h>


// Motor 1 (Left)
#define D1IN1 14
#define D1IN2 15
#define D1ENA 5  // PWM Speed


// Motor 2 (Right)
#define D1IN3 16
#define D1IN4 17
#define D1ENB 6  // PWM Speed

#define TURN_SPEED  70
#define GYRO_HPF    0.05f  // Ignore gyro readings below this
#define TURN_BRAKE_OFFSET 0.0f   // degrees to stop early, tune empirically
#define TURN_TIMEOUT_US 5000000UL

void initmotors();
void drive(int leftspeed, int rightspeed);
void motorstop();
void turndegrees(float targetDegrees);
void driveforward(int durationMS);


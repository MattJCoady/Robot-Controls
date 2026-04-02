#pragma once
#include <Arduino.h>
#include <SPI.h>

#define CS_PIN 10

#define BMI323_REG_CHIP_ID       0x00
#define BMI323_REG_STATUS        0x02
#define BMI323_REG_ACC_DATA_X    0x03
#define BMI323_REG_GYR_DATA_X    0x06
#define BMI323_REG_ACC_CONF      0x20
#define BMI323_REG_GYR_CONF      0x21
#define BMI323_REG_CMD           0x7E

#define BMI323_SPI_READ          0x80

#define ACCEL_SCALE              (8.0f / 32768.0f)
#define GYRO_SCALE               (2000.0f / 32768.0f)

struct IMUData {
  float ax, ay, az;
  float gx, gy, gz;
};

bool bmi323Init();
IMUData readIMU();

extern float heading;
extern unsigned long lastIMUTime;

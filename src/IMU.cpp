#include <IMU.h>

float heading = 0;
unsigned long lastIMUTime = 0;

SPISettings bmi323Settings(1000000, MSBFIRST, SPI_MODE0);

// ---------------------------------------------------------------
// Low-level SPI helpers
// ---------------------------------------------------------------

void writeRegister16(uint8_t reg, uint16_t value) {
  SPI.beginTransaction(bmi323Settings);
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(5);
  SPI.transfer(reg & 0x7F);
  SPI.transfer(value & 0xFF);
  SPI.transfer((value >> 8) & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  delayMicroseconds(10);
}

uint16_t readRegister16(uint8_t reg) {
  SPI.beginTransaction(bmi323Settings);
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(5);
  SPI.transfer(reg | BMI323_SPI_READ);
  SPI.transfer(0x00); // Dummy byte
  uint8_t low  = SPI.transfer(0x00);
  uint8_t high = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  delayMicroseconds(10);
  return ((uint16_t)high << 8) | low;
}

void readRegisters(uint8_t reg, uint16_t *buf, uint8_t count) {
  SPI.beginTransaction(bmi323Settings);
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(5);
  SPI.transfer(reg | BMI323_SPI_READ);
  SPI.transfer(0x00); // Dummy byte
  for (uint8_t i = 0; i < count; i++) {
    uint8_t low  = SPI.transfer(0x00);
    uint8_t high = SPI.transfer(0x00);
    buf[i] = ((uint16_t)high << 8) | low;
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  delayMicroseconds(10);
}

// ---------------------------------------------------------------
// BMI323 Initialization
// ---------------------------------------------------------------

bool bmi323Init() {
  // Step 1: Soft reset
  writeRegister16(BMI323_REG_CMD, 0xDEAF);
  delay(10);

  // Step 2: Dummy read required after reset to init SPI interface
  readRegister16(BMI323_REG_CHIP_ID);
  delayMicroseconds(100);

  // Step 3: Read chip ID for real
  uint16_t chipId = readRegister16(BMI323_REG_CHIP_ID);
  Serial.print("Chip ID: 0x");
  Serial.println(chipId & 0xFF, HEX);

  if ((chipId & 0xFF) != 0x43) {
    Serial.println("ERROR: BMI323 not found!");
    return false;
  }
  Serial.println("Chip ID verified OK");

  // Step 4: Enable accelerometer — normal mode, ±8g, 100Hz
  writeRegister16(BMI323_REG_ACC_CONF, 0x4028);
  delay(50);

  // Step 5: Enable gyroscope — normal mode, ±2000dps, 100Hz
  writeRegister16(BMI323_REG_GYR_CONF, 0x4028);
  delay(50);

  // Step 6: Verify accel came out of suspend
  uint16_t accX = readRegister16(BMI323_REG_ACC_DATA_X);
  if (accX == 0x8000) {
    Serial.println("WARNING: Accel still in suspend after config — check ACC_CONF bits");
  }

  Serial.println("BMI323 initialized successfully.\n");
  return true;
}

// ---------------------------------------------------------------
// IMU Data Reading
// ---------------------------------------------------------------

IMUData readIMU() {
  uint16_t raw[6];
  readRegisters(BMI323_REG_ACC_DATA_X, raw, 6);

  IMUData data;
  data.ax = (int16_t)raw[0] * ACCEL_SCALE;
  data.ay = (int16_t)raw[1] * ACCEL_SCALE;
  data.az = (int16_t)raw[2] * ACCEL_SCALE;
  data.gx = (int16_t)raw[3] * GYRO_SCALE;
  data.gy = (int16_t)raw[4] * GYRO_SCALE;
  data.gz = (int16_t)raw[5] * GYRO_SCALE;
  return data;
}
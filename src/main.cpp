#include <Arduino.h>
#include<DriveTrain.h>
#include<Sensors.h>
#include<IMU.h>
#include<Bluetooth.h>

enum robotstate {IDLE, GOING_TO_TARGET, AT_TARGET, RETURNING_HOME};
robotstate currentState = IDLE;

void setup() {
  Serial.begin(9600);
  
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  delay(100);

  initmotors();
  initsensors();
  
  if (!bmi323Init()) {
    Serial.println("Halted. Fix IMU then reset.");
    while (1);
  }

  lastIMUTime = micros(); // seed the heading timer
  sendBluetooth("Ready.");
}


void loop() {
  char cmd = 0;
  if (bluetoothAvailable()) {
    cmd = readBluetooth();
    if (cmd == 'E' || cmd == 'e') {
      motorstop();
      currentState = IDLE;
      sendBluetooth("EMERGENCY STOP.");
      return;
    }
  }

  switch (currentState) {

    case IDLE:
      if (cmd == 'S' || cmd == 's') {
        currentState = GOING_TO_TARGET;
      }
      break;

    case GOING_TO_TARGET:
      turndegrees(180);
      currentState = AT_TARGET;
      sendBluetooth("Arrived at target, send C to confirm handoff.");
      break;

    case AT_TARGET:
      if (cmd == 'C' || cmd == 'c') {
        currentState = RETURNING_HOME;
      }
      break;

    case RETURNING_HOME:
      currentState = IDLE;
      sendBluetooth("Returned home, send S to start.");
      break;
  }
}
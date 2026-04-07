#include <Arduino.h>
#include<DriveTrain.h>
#include<Sensors.h>
#include<IMU.h>
#include<Bluetooth.h>

enum robotstate {IDLE, GOING_TO_TARGET, AT_TARGET, RETURNING_HOME,TURNING};
robotstate currentState = IDLE;

unsigned long powerlab_home_timer = 0;
unsigned long hall_powerlab_timer = 0;
unsigned long incalab_hall_timer = 0;

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

  lastIMUTime = micros(); // Set the heading timer
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
      if (cmd == 'T' || cmd == 't') {
        currentState = TURNING;
      }
      break;

    case TURNING:{
      if (cmd == 'l' || cmd == 'L') {
        sendBluetooth("Turning 90 deg Left...");
        turndegrees(-90); 
        
        sendBluetooth("Ready. l:90L, r:90R, q:180L, p:180R");
      }
      else if (cmd == 'r' || cmd == 'R') {
        sendBluetooth("Turning 90 deg Right...");
        turndegrees(90);
        
        sendBluetooth("Ready. l:90L, r:90R, q:180L, p:180R");
      }
      else if (cmd == 'q' || cmd == 'Q') {
        sendBluetooth("Turning 180 deg Left...");
        turndegrees(-180);
        
        sendBluetooth("Ready. l:90L, r:90R, q:180L, p:180R");
      }
      else if (cmd == 'p' || cmd == 'P') {
        sendBluetooth("Turning 180 deg Right...");
        turndegrees(180);
        
        sendBluetooth("Ready. l:90L, r:90R, q:180L, p:180R");
      }
      
      break;
    }
    
    case GOING_TO_TARGET:
    {
      heading = 0.0;
      targetHeading = heading; // Set the straight heading
      lastIMUTime = micros();
      unsigned long driveTime1 = 0;
      unsigned long driveTime2 = 0;
      unsigned long driveTime3 = 0; 
      bool wallDetected = false;
      unsigned long driveStartTime = millis(); // Start the drive stopwatch
      
      sendBluetooth("going to doorway");

      while (!wallDetected) {

        // 1. Is there an object in the way
        if (obstacleDetected()) {

            motorstop();
            // Save the time  spent driving so far
            driveTime1 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch 
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {
            // 2. Path is clear, keep the motors running straight
            driveforwardUT(150);
        }
      }
      powerlab_home_timer = driveTime1;

      motorstop();
      sendBluetooth("Wall detected! Turning left");
      delay(500);

      // 3. Do a -90 degree turn
      turndegrees(-90);
      lastIMUTime = micros();
      targetHeading = targetHeading - 90;

      wallDetected = false;
      driveStartTime = millis();

      sendBluetooth ("Going through hallway");
      while (!wallDetected) {

        if (obstacleDetected()) {
            
            motorstop();
            driveTime2 += (millis() - driveStartTime);
        
            wallDetected = handleObstacles(); 
            
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {
 
            driveforwardUT(150);
        }
      }
      hall_powerlab_timer = driveTime2;

      motorstop();

      turndegrees(90);
      lastIMUTime = micros();
      targetHeading = targetHeading + 90;
      wallDetected = false;
      driveStartTime = millis();

      sendBluetooth("Going into INCA lab");
      while (!wallDetected) {

        if (obstacleDetected()) {
            
            motorstop();
           
            driveTime3 += (millis() - driveStartTime);

            wallDetected = handleObstacles(); 
            
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {

            driveforwardUT(150);
        }
      }
      incalab_hall_timer = driveTime3;

      motorstop();

      currentState = AT_TARGET;
      sendBluetooth("Arrived at target, send C to confirm handoff.");
      break;
    }
    case AT_TARGET:
      if (cmd == 'C' || cmd == 'c') {
        currentState = RETURNING_HOME;
      }
      break;

    case RETURNING_HOME:
    {
      turndegrees(+180);
      lastIMUTime = micros();
      targetHeading = targetHeading + 180;

      bool wallDetected = false;
      unsigned long driveStartTime = millis();
      unsigned long totalDriveTime = 0;

      while (!wallDetected) {
        totalDriveTime = millis() - driveStartTime;

        if (totalDriveTime >= incalab_hall_timer) break;

        if (obstacleDetected()) {
        
          motorstop();
          unsigned long pauseStart = millis();
          wallDetected = handleObstacles(); 

          driveStartTime += (millis() - pauseStart);
          lastIMUTime = micros();
        } 

        else {driveforwardUT(150);}
      }

      motorstop();
      delay(200);
      turndegrees(-90);

      lastIMUTime = micros();
      targetHeading = targetHeading - 90;
      wallDetected = false;
      driveStartTime = millis();
      totalDriveTime = 0;

      while (!wallDetected) {
        totalDriveTime = millis() - driveStartTime;

          if (totalDriveTime >= hall_powerlab_timer) break;

          if (obstacleDetected()) {
        
            motorstop();
            unsigned long pauseStart = millis();
            wallDetected = handleObstacles(); 

            driveStartTime += (millis() - pauseStart);
            lastIMUTime = micros();
          } 

        else {driveforwardUT(150);}
      }

      motorstop();
      delay(200);
      turndegrees(90);

      lastIMUTime = micros();
      targetHeading = targetHeading + 90;
      wallDetected = false;
      driveStartTime = millis();
      totalDriveTime = 0;

      while (!wallDetected) {
        totalDriveTime = millis() - driveStartTime;

          if (totalDriveTime >= powerlab_home_timer) break;

          if (obstacleDetected()) {
        
            motorstop();
            unsigned long pauseStart = millis();
            wallDetected = handleObstacles(); 

            driveStartTime += (millis() - pauseStart);
            lastIMUTime = micros();
          } 

        else {driveforwardUT(150);}
      }
      
      motorstop();
      delay(200);
      turndegrees(180);

      lastIMUTime = micros();
      targetHeading = targetHeading + 180;

      currentState = IDLE;
      sendBluetooth("Returned home, send S to start.");
      break;
    }
  }
}
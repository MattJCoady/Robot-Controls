#include <Arduino.h>
#include<DriveTrain.h>
#include<Sensors.h>
#include<IMU.h>
#include<Bluetooth.h>

enum robotstate {IDLE, GOING_TO_TARGET, AT_TARGET, RETURNING_HOME, DOOR_AND_BACK, TURNING};
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
      if (cmd == 'B' || cmd == 'b') {
        currentState = DOOR_AND_BACK;
      }
      if (cmd == 'T' || cmd == 't') {
        currentState = TURNING;
      }
      break;

    case TURNING:{
      // If we received a valid command, execute the turn
      if (cmd == 'l' || cmd == 'L') {
        sendBluetooth("Turning 90 deg Left...");
        turndegrees(-90);  // Positive is usually Left
        
        // Re-print the menu after finishing
        sendBluetooth("Ready. l:90L, r:90R, t:360L, p:360R");
      }
      else if (cmd == 'r' || cmd == 'R') {
        sendBluetooth("Turning 90 deg Right...");
        turndegrees(90); // Negative is usually Right
        
        sendBluetooth("Ready. l:90L, r:90R, t:360L, p:360R");
      }
      else if (cmd == 'q' || cmd == 'Q') {
        sendBluetooth("Turning 360 deg Left...");
        turndegrees(-180);
        
        sendBluetooth("Ready. l:90L, r:90R, t:360L, p:360R");
      }
      else if (cmd == 'p' || cmd == 'P') {
        sendBluetooth("Turning 360 deg Right...");
        turndegrees(180);
        
        sendBluetooth("Ready. l:90L, r:90R, t:360L, p:360R");
      }
      
      // If no valid command was sent, it just breaks and loops again, 
      // waiting silently for the next command!
      break;
    }
    

    
    
    case DOOR_AND_BACK:
    {
      targetHeading = heading; // Lock in the straight heading
      unsigned long totalDriveTime = 0; 
      bool wallDetected = false;
      
      unsigned long driveStartTime = millis(); // Start the drive stopwatch
      
      sendBluetooth("Driving to door...");

    while (!wallDetected) {
        // 1. Is there an object in the way?
        if (obstacleDetected()) {
            // Pause the stopwatch! Save the time we spent driving so far.
            totalDriveTime += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
            // DON'T FORGET THIS!
            lastIMUTime = micros();
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(150);
        }
      }
      sendBluetooth("Wall detected! Turning around...");
      delay(500); // Brief pause to let physical momentum settle

      // 3. Do a 180 degree turn
      turndegrees(-180);

      // 4. Drive back to the start using the exact time we accumulated
      sendBluetooth("Returning home...");
      driveforward(totalDriveTime, 150); 

      // 5. Spin 180 to face original orientation (optional, but good practice)
      turndegrees(-180);

      currentState = IDLE;
      sendBluetooth("Returned home. Send S to start.");
      break;
    }

    case GOING_TO_TARGET:
    {
      heading = 0.0;
      targetHeading = heading; // Lock in the straight heading
      lastIMUTime = micros();
      unsigned long driveTime1 = 0;
      unsigned long driveTime2 = 0;
      unsigned long driveTime3 = 0; 
      bool wallDetected = false;
      unsigned long driveStartTime = millis(); // Start the drive stopwatch
      
      sendBluetooth("going to doorway");

      while (!wallDetected) {

        // 1. Is there an object in the way?
        if (obstacleDetected()) {

            motorstop();
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime1 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(150);
        }
      }
      powerlab_home_timer = driveTime1;

      motorstop();
      sendBluetooth("Wall detected! Turning left");
      delay(500); // Brief pause to let physical momentum settle

      // 3. Do a -90 degree turn
      turndegrees(-90);
      lastIMUTime = micros();
      targetHeading = targetHeading - 90;

      wallDetected = false;
      driveStartTime = millis();

      sendBluetooth ("going through hallway");
      while (!wallDetected) {

        // 1. Is there an object in the way?
        if (obstacleDetected()) {
            
            motorstop();
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime2 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {
            // 2. Path is clear, keep the motors running straight!
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

      sendBluetooth("going into INCA lab");
      while (!wallDetected) {

        // 1. Is there an object in the way?
        if (obstacleDetected()) {
            
            motorstop();
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime3 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 

            lastIMUTime = micros();
            
        } else {
            // 2. Path is clear, keep the motors running straight!
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
      turndegrees(180);
      
      lastIMUTime = micros();
      targetHeading = targetHeading + 180;

      currentState = IDLE;
      sendBluetooth("Returned home, send S to start.");
      break;
    }
  }
}
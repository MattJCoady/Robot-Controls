#include <Arduino.h>
#include<DriveTrain.h>
#include<Sensors.h>
#include<IMU.h>
#include<Bluetooth.h>

enum robotstate {IDLE, GOING_TO_TARGET, AT_TARGET, RETURNING_HOME, DOOR_AND_BACK};
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
      if (cmd == 'B' || cmd == 'b') {
        currentState = DOOR_AND_BACK;
      }
    
      break;
    
    
    case DOOR_AND_BACK:
    {
      targetHeading = heading; // Lock in the straight heading
      unsigned long totalDriveTime = 0; 
      bool wallDetected = false;
      
      unsigned long driveStartTime = millis(); // Start the drive stopwatch
      
      sendBluetooth("Driving to door...");

      while (!wallDetected) {
        float dist = getdistance();

        // 1. Is there an object in the way?
        if (dist > 0 && dist < 25.0) {
            
            // Pause the stopwatch! Save the time we spent driving so far.
            totalDriveTime += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(100);
        }
      }

      motorstop();
      sendBluetooth("Wall detected! Turning around...");
      delay(500); // Brief pause to let physical momentum settle

      // 3. Do a 180 degree turn
      turndegrees(-180);

      // 4. Drive back to the start using the exact time we accumulated
      sendBluetooth("Returning home...");
      driveforward(totalDriveTime, 100); 

      // 5. Spin 180 to face original orientation (optional, but good practice)
      turndegrees(-180);

      currentState = IDLE;
      sendBluetooth("Returned home. Send S to start.");
      break;
    }

    case GOING_TO_TARGET:
    {
      targetHeading = heading; // Lock in the straight heading
      unsigned long driveTime1 = 0;
      unsigned long driveTime2 = 0;
      unsigned long driveTime3 = 0; 
      bool wallDetected = false;
      unsigned long powerlab_home_timer = 0;
      unsigned long hall_powerlab_timer = 0;
      unsigned long incalab_hall_timer = 0;
      
      unsigned long driveStartTime = millis(); // Start the drive stopwatch
      
      sendBluetooth("going to doorway");

      while (!wallDetected) {
        float dist = getdistance();

        // 1. Is there an object in the way?
        if (dist > 0 && dist < 25.0) {
            
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime1 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(100);
        }
      }
      powerlab_home_timer = driveTime1;

      motorstop();
      sendBluetooth("Wall detected! Turning left");
      delay(500); // Brief pause to let physical momentum settle

      // 3. Do a -90 degree turn
      turndegrees(-90);

      sendBluetooth ("going through hallway");
      while (!wallDetected) {
        float dist = getdistance();

        // 1. Is there an object in the way?
        if (dist > 0 && dist < 25.0) {
            
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime2 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(100);
        }
      }
      hall_powerlab_timer = driveTime2;

      motorstop();

      turndegrees(90);
      sendBluetooth("going into INCA lab");

      while (!wallDetected) {
        float dist = getdistance();

        // 1. Is there an object in the way?
        if (dist > 0 && dist < 25.0) {
            
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime1 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(100);
        }
      }
      powerlab_home_timer = driveTime1;

      motorstop();
      sendBluetooth("Wall detected! Turning left");
      delay(500); // Brief pause to let physical momentum settle

      // 3. Do a -90 degree turn
      turndegrees(-90);

      sendBluetooth ("going through hallway");
      while (!wallDetected) {
        float dist = getdistance();

        // 1. Is there an object in the way?
        if (dist > 0 && dist < 25.0) {
            
            // Pause the stopwatch! Save the time we spent driving so far.
            driveTime3 += (millis() - driveStartTime);
            
            // This stops the motors and waits. Returns true if it's a solid wall.
            wallDetected = handleObstacles(); 
            
            // Restart the stopwatch (If it was just a person, we are about to resume driving)
            driveStartTime = millis(); 
            
        } else {
            // 2. Path is clear, keep the motors running straight!
            driveforwardUT(100);
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
      currentState = IDLE;
      sendBluetooth("Returned home, send S to start.");
      break;
  }
}
// Note tried the door and back function 
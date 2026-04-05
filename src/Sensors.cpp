#include<Arduino.h>
#include<Sensors.h>
#include<DriveTrain.h>

void initsensors() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

long getdistance()
{
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    long duration = pulseIn(PIN_ECHO, HIGH, 20000);
    if (duration == 0) {
      return 999;
    }
    return duration/58.0; 
}

bool rightBlocked() {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(IR_RIGHT) == LOW) count++;
    delay(1);
  }
  return count >= 7;
}

bool leftBlocked() {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(IR_LEFT) == LOW) count++;
    delay(1);
  }
  return count >= 7;
}

bool handleObstacles() {
    unsigned long waitStart = millis();

    while (true) {
        if (millis() - waitStart > 3000) {
            return true;
        }
        int clearCount = 0;
        for (int i = 0; i < 5; i++) {
            if (getdistance() > 20.0) clearCount++;
            delay(10);
        }
        if (clearCount >= 4) {
            return false;
        }
    }
}

bool obstacleDetected() {
  static unsigned long lastPingTime = 0;
  if (millis() - lastPingTime > 50){
    lastPingTime = millis();
    float d = getdistance();
    if ((d > 0 && d < 20.0) || leftBlocked() || rightBlocked()) { //may need to comment out ir sensors when demoing
      return true;
    }
  }
  return false;
}
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
    long duration = pulseIn(PIN_ECHO, HIGH);
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

// void handleObstacles() {
//     float distance = getdistance();
//     if (distance > 0 && distance < 25.0){
//         motorstop();
//         while (getdistance() < 25.0){
//             delay(50);
//             //timeout logic goes here
//         }
//     }
// }
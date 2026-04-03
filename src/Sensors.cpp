#include<Arduino.h>
#include<Sensors.h>

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
    delay(2);
  }
  return count >= 7;
}

bool leftBlocked() {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(IR_LEFT) == LOW) count++;
    delay(2);
  }
  return count >= 7;
}
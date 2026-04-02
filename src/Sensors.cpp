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

bool leftclear()
{
    int leftDetected = digitalRead(IR_LEFT);
    if (leftDetected == LOW) {return false;} 
    else {return true;}
}

bool rightclear()
{
    int rightDetected = digitalRead(IR_RIGHT);
    if (rightDetected == LOW) {return false;} 
    else {return true;}
}
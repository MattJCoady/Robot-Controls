#pragma once
#include<Arduino.h>



#define IR_LEFT 18
#define IR_RIGHT 19
#define PIN_TRIG 2
#define PIN_ECHO 3


void initsensors();
long getdistance();
bool leftBlocked();
bool rightBlocked();
// void handleObstacles();
#pragma once
#include <Arduino.h>

void initbluetooth();
bool bluetoothAvailable();
char readBluetooth();
void sendBluetooth(const char* message);

#pragma once
#include <Arduino.h>

bool bluetoothAvailable();
char readBluetooth();
void sendBluetooth(const char* message);

#include <Bluetooth.h>

bool bluetoothAvailable() {
  return Serial.available() > 0;
}

char readBluetooth() {
  return (char)Serial.read();
}

void sendBluetooth(const char* message) {
  Serial.println(message);
}

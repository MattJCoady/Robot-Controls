#include <Bluetooth.h>

void initbluetooth() {
  //HC-05 default baud rate is 9600
  // To change HC-05 baud to 115200: enter AT mode and send AT+UART=115200,1,0
}

bool bluetoothAvailable() {
  return Serial.available() > 0;
}

char readBluetooth() {
  return (char)Serial.read();
}

void sendBluetooth(const char* message) {
  Serial.println(message);
}

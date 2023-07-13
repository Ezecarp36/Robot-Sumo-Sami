#include <AnalogSensor.h>
#include "BluetoothSerial.h"


#define PIN_SENSOR_LDR 23
#define MONTADO 100
int ldrReading;

unsigned long currentTime = 0;
#define TICK_DEBUG 500

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

AnalogSensor *sensorLdr = new AnalogSensor(PIN_SENSOR_LDR);

void setup() {
  SerialBT.begin("Sami");
  pinMode(PIN_SENSOR_LDR, INPUT);
  Serial.begin(9600);
}

void loop() {
  ldrReading = analogRead(PIN_SENSOR_LDR);
  if (millis() > currentTime + TICK_DEBUG) {
    Serial.print("ldr reading: ");
    Serial.println(ldrReading);
  }
}
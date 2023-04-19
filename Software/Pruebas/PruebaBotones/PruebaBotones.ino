#include <Button.h>
#include "BluetoothSerial.h"

#define PIN_BUTTON_START 34
#define PIN_BUTTON_STRATEGY 35

Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_START);

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

  void setup() {
  SerialBT.begin("Sami");
}

void loop() {
  if (buttonStrategy->GetIsPress()) SerialBT.print("Button strategy is press");
  if (buttonStart->GetIsPress()) SerialBT.print("Button start is press");
  else SerialBT.print("----------------");
}

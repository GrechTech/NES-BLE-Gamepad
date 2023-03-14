#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

static const int LIGHT_PIN = 21;	
static const int TRIGGER_PIN = 19;

BleGamepad bleGamepad("NES-Zapper", "GrechTech", 100);

void setup()
{
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  bleGamepad.begin();
}

void loop()
{
  if (bleGamepad.isConnected())
  {
    if(digitalRead(LIGHT_PIN))
    {
      bleGamepad.press(BUTTON_1);
    }
    else
    {
      bleGamepad.release(BUTTON_1);
    }

    if(digitalRead(TRIGGER_PIN))
    {
      bleGamepad.press(BUTTON_2);
    }
    else
    {
      bleGamepad.release(BUTTON_2);
    }
  }
}
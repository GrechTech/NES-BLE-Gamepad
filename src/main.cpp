#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

static const int LIGHT_PIN = 21;	// ESP32 IO pins
static const int TRIGGER_PIN = 19; // any input pins with pullups will work

BleGamepad bleGamepad("NES-Zapper", "GrechTech", 100); // Initialise Bluetooth gamepad

void setup()
{
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up

  bleGamepad.begin();
}

void loop()
{
  if (bleGamepad.isConnected())
  {
    if(!digitalRead(LIGHT_PIN)) //Inverted Light Pin
      bleGamepad.press(BUTTON_1);
    else
      bleGamepad.release(BUTTON_1);

    if(digitalRead(TRIGGER_PIN))
      bleGamepad.press(BUTTON_2);
    else
      bleGamepad.release(BUTTON_2);
  }
}
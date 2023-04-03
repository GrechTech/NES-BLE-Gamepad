#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

//#define DEBUG

static const int LIGHT_PIN = 21;	// ESP32 IO pins
static const int TRIGGER_PIN = 19; // any input pins with pullups will work

bool Light = true;
bool Trigger = false;
bool Changed = false;

BleGamepad bleGamepad("NES-Zapper", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Create a BleGamepadConfiguration object to store all of the options

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.print("Start");
#endif

  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up

  bleGamepadConfig.setButtonCount(12); 
  bleGamepadConfig.setHatSwitchCount(1);
  bleGamepadConfig.setIncludeZAxis(false); // Simplify the HID report 
  bleGamepadConfig.setIncludeRxAxis(false);
  bleGamepadConfig.setIncludeRyAxis(false);
  bleGamepadConfig.setIncludeRzAxis(false);
  bleGamepadConfig.setIncludeSlider1(false);
  bleGamepadConfig.setIncludeSlider2(false);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);

  //Send first report
  bleGamepad.release(BUTTON_1);
  bleGamepad.release(BUTTON_2);
  bleGamepad.sendReport();
}

void loop()
{
  if (bleGamepad.isConnected())
  {
    Changed = false;

    if(digitalRead(LIGHT_PIN) && !Light) 
    {
      Light = true;
      bleGamepad.release(BUTTON_1); //Inverted Light Pin
      Changed = true;
      #ifdef DEBUG
        Serial.println("Light Off");
      #endif
    }
    else if(!digitalRead(LIGHT_PIN) && Light) 
    {
      Light = false;
      bleGamepad.press(BUTTON_1);
      Changed = true;
      #ifdef DEBUG
        Serial.println("Light On");
      #endif
    }

    if(digitalRead(TRIGGER_PIN) && !Trigger)
    {
      Trigger = true;
      bleGamepad.press(BUTTON_2);
      Changed = true;
      #ifdef DEBUG
        Serial.println("Trigger On");
      #endif
    }
    else if(!digitalRead(TRIGGER_PIN) && Trigger)
    {
      Trigger = false;
      bleGamepad.release(BUTTON_2);
      Changed = true;
      #ifdef DEBUG
        Serial.println("Trigger Off");
      #endif
    }

    if (Changed)
    {
      bleGamepad.sendReport();
      #ifdef DEBUG
        Serial.print(".");
      #endif
    }
  }
}
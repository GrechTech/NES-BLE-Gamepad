#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

static const int LIGHT_PIN = 21;	// ESP32 IO pins
static const int TRIGGER_PIN = 19; // any input pins with pullups will work

BleGamepad bleGamepad("NES-Zapper", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Create a BleGamepadConfiguration object to store all of the options

void setup()
{
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up
  
  bleGamepadConfig.setButtonCount(2); // Simplify the HID report 
  bleGamepadConfig.setHatSwitchCount(0); // to just the 2 buttons required
  bleGamepadConfig.setIncludeXAxis(false); // with no HAT or analogue inputs
  bleGamepadConfig.setIncludeYAxis(false);
  bleGamepadConfig.setIncludeZAxis(false);
  bleGamepadConfig.setIncludeRxAxis(false);
  bleGamepadConfig.setIncludeRyAxis(false);
  bleGamepadConfig.setIncludeRzAxis(false);
  bleGamepadConfig.setIncludeSlider1(false);
  bleGamepadConfig.setIncludeSlider2(false);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);
  Serial.begin(115200);
  Serial.println("Start");
}

bool LIGHT_STATE = false;	// ESP32 IO pins
bool TRIGGER_STATE = false; // any input pins with pullups will work
bool STATE_CHANGE = false; // any input pins with pullups will work
bool connected = false;

void loop()
{
  if (bleGamepad.isConnected())
  {
    if(!connected)
    {
      Serial.println("Connected");
      connected = true;
    }

    STATE_CHANGE = false;
    bool Light = digitalRead(LIGHT_PIN);
    bool Trigger = digitalRead(TRIGGER_PIN);

    // Check if a state has changed
    if( (Light != LIGHT_STATE) || (Trigger != TRIGGER_STATE) )
        STATE_CHANGE = true;

    LIGHT_STATE = Light;
    TRIGGER_STATE = Trigger;

    if(STATE_CHANGE) // Only send report if state changed
    {
      //Send outputs
      if(!LIGHT_STATE) //Inverted Light Pin
      {
        bleGamepad.press(BUTTON_2);
        Serial.println("Light On");
      }
      else
        bleGamepad.release(BUTTON_2);

      if(!TRIGGER_STATE)    //Normal operation //invert for latency test
      {
        bleGamepad.press(BUTTON_1);
        Serial.println("Trigger On");
      }
      else
        bleGamepad.release(BUTTON_1);

      bleGamepad.sendReport();
    }
  }
  else
  {
    
    if(connected)
    {
      Serial.println("Disconnected");
      connected = false;
    }
    
  }
}
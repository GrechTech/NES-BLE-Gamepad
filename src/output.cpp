#include "main.h"
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

const uint8_t PowerPadBtnMap [12] = {1, 0, 4, 8, 5, 9, 10, 6, 3, 2, 11, 7}; // Remap shift register value to PowerPad label
const uint8_t SELECT = BUTTON_5;    // Alternate Select mapping for emulator mode
const uint8_t B_BUTTON = BUTTON_2 + ((uint8_t)EMULATOR_MAPPING * B_BUTTON_EMU_OFFSET);  // B button mapping

BleGamepad bleGamepad("NES Controller", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Store all of the Bluetooth options

bool connected() // Check if Bluetooth connected
{
  return bleGamepad.isConnected();
}

inline void outputDirect(bool press, uint8_t input) // Output a button directly
{
  if(press)
    bleGamepad.press(input);
  else
    bleGamepad.release(input);

  if (DEBUG)
  {
    Serial.print("# BTN: ");
    Serial.print(input);
    if(press)
      Serial.println(" Pressed");
    else
      Serial.println(" Released");
  }
}

void setupBluetooth() // Setup the Bluetooth gamepad service
{
  bleGamepad.deviceName = "NES BLE Gamepad";
  bleGamepadConfig.setHatSwitchCount(1);    // Set a single D-Pad
  bleGamepadConfig.setIncludeZAxis(false);  // Simplify the HID report 
  bleGamepadConfig.setIncludeRxAxis(false); // By removing unused axis
  bleGamepadConfig.setIncludeRyAxis(false);
  bleGamepadConfig.setIncludeRzAxis(false);
  bleGamepadConfig.setIncludeSlider1(false);
  bleGamepadConfig.setIncludeSlider2(false);

  bleGamepadConfig.setButtonCount(12);  

  bleGamepadConfig.setIncludeStart(true);
  bleGamepadConfig.setIncludeSelect(true);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);

  if(zapperPad)
    outputDirect(true,B_BUTTON);

  DebugOut("##### Done Setup BLE");

  bleGamepad.sendReport();
}

inline void pressSelect(bool input) // Press the select button
{
  if(input)
  {
    if(EMULATOR_MAPPING) // If using alternate Select mapping
      bleGamepad.press(SELECT);
    else
      bleGamepad.pressSelect();
  }
  else
  {
    if(EMULATOR_MAPPING) // If using alternate Select mapping
      bleGamepad.release(SELECT);
    else
      bleGamepad.releaseSelect();
  }
}

inline void outputGamepad(uint8_t gamepadData, uint8_t prevPadData) // Output using gamepad value data
{
  if(gamepadData != prevPadData) // If state changed
  {
    if((bitRead(gamepadData,  7 - 0) == LOW) && (bitRead(prevPadData, 7 -  0) == HIGH))  // Inverted
    {
      bleGamepad.press(BUTTON_1);
      DebugOut("# BTN: A Pressed");
    }
    else if((bitRead(gamepadData, 7 - 0) == HIGH) && (bitRead(prevPadData, 7 - 0) == LOW)) 
    {
      bleGamepad.release(BUTTON_1);
      DebugOut("# BTN: A Released");
    }

    if((bitRead(gamepadData, 7 - 1) == LOW) && (bitRead(prevPadData, 7 - 1) == HIGH)) 
    {
      bleGamepad.press(B_BUTTON);
      DebugOut("# BTN: B Pressed");
    }  
    else if((bitRead(gamepadData, 7 - 1) == HIGH) && (bitRead(prevPadData, 7 - 1) == LOW)) 
    {
      bleGamepad.release(B_BUTTON);
      DebugOut("# BTN: B Released");
    }

    if((bitRead(gamepadData, 7 - 2) == LOW) && (bitRead(prevPadData, 7 - 2) == HIGH)) 
    {
      pressSelect(true);
      DebugOut("# BTN: Select Pressed");
    }
    else if((bitRead(gamepadData, 7 - 2) == HIGH) && (bitRead(prevPadData, 7 - 2) == LOW)) 
    {
      pressSelect(false);
      DebugOut("# BTN: Select Released");
    }

    if((bitRead(gamepadData, 7 - 3) == LOW) && (bitRead(prevPadData, 7 - 3) == HIGH))
    {
      bleGamepad.pressStart();
      DebugOut("# BTN: Start Pressed");
      
    }
    else if((bitRead(gamepadData, 7 - 3) == HIGH) && (bitRead(prevPadData, 7 - 3) == LOW))
    {
      bleGamepad.releaseStart();
      DebugOut("# BTN: Start Released");
    }

    // Check if DPad changed
    if( (bitRead(gamepadData, 7 - 4) != bitRead(prevPadData, 7 - 4)) 
      || (bitRead(gamepadData, 7 - 5) != bitRead(prevPadData, 7 - 5))
      || (bitRead(gamepadData, 7 - 6) != bitRead(prevPadData, 7 - 6)) 
      || (bitRead(gamepadData, 7 - 7) != bitRead(prevPadData, 7 - 7))) 
    {
      DebugOut("# DPAD: ", false);

      if( (bitRead(gamepadData, 7 - 4) == LOW) && (bitRead(gamepadData, 7 - 5) == HIGH)
      && (bitRead(gamepadData, 7 - 6) == HIGH) && (bitRead(gamepadData, 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat(HAT_UP); // UP          (N)
        DebugOut("1 U");
      }
      else if( (bitRead(gamepadData, 7 - 4) == LOW) && (bitRead(gamepadData, 7 - 5) == HIGH)
            && (bitRead(gamepadData, 7 - 6) == HIGH) && (bitRead(gamepadData, 7 - 7) == LOW)) 
      {
        bleGamepad.setHat(HAT_UP_RIGHT); // UP RIGHT    (NE)
        DebugOut("2 UR");
      }
      else if( (bitRead(gamepadData, 7 - 4) == HIGH) && (bitRead(gamepadData, 7 - 5) == HIGH)
            && (bitRead(gamepadData, 7 - 6) == HIGH) && (bitRead(gamepadData, 7 - 7) == LOW)) 
      {
        bleGamepad.setHat(HAT_RIGHT); // RIGHT       (E)
        DebugOut("3 R");
      }
      else if( (bitRead(gamepadData, 7 - 4) == HIGH) && (bitRead(gamepadData, 7 - 5) == LOW)
            && (bitRead(gamepadData, 7 - 6) == HIGH) && (bitRead(gamepadData, 7 - 7) == LOW)) 
      {
        bleGamepad.setHat(HAT_DOWN_RIGHT); // DOWN RIGHT  (SE)
        DebugOut("4 DR");
      }
      else if( (bitRead(gamepadData, 7 - 4) == HIGH) && (bitRead(gamepadData, 7 - 5) == LOW)
            && (bitRead(gamepadData, 7 - 6) == HIGH) && (bitRead(gamepadData, 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat(HAT_DOWN); // DOWN        (S)
        DebugOut("5 D");
      }
      else if( (bitRead(gamepadData, 7 - 4) == HIGH) && (bitRead(gamepadData, 7 - 5) == LOW)
            && (bitRead(gamepadData, 7 - 6) == LOW) && (bitRead(gamepadData, 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat(HAT_DOWN_LEFT);  // DOWN LEFT  (SW)
        DebugOut("6 DL");
      }
      else if( (bitRead(gamepadData, 7 - 4) == HIGH) && (bitRead(gamepadData, 7 - 5) == HIGH)
            && (bitRead(gamepadData, 7 - 6) == LOW) && (bitRead(gamepadData, 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat(HAT_LEFT); // LEFT        (W)
        DebugOut("7 L");
      }
      else if( (bitRead(gamepadData, 7 - 4) == LOW) && (bitRead(gamepadData, 7 - 5) == HIGH)
            && (bitRead(gamepadData, 7 - 6) == LOW) && (bitRead(gamepadData, 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat(HAT_UP_LEFT); // UP LEFT     (NW)
        DebugOut("8 UL");
      }
      else
      {
        bleGamepad.setHat(HAT_CENTERED); // CENTRE      (C)
        DebugOut("0 C");
      }
    }

    bleGamepad.sendReport();

    if (DEBUG)
      Serial.println(gamepadData, BIN);
  }
}

inline void outputPowerpad(uint16_t powerpadData, uint16_t prevPadData) // Output using powerpad value data
{
  if(COMPRESS_POWERPAD) // If using the compressed scheme for NESLCD ROM patches
  {
    for(uint8_t n = 0; n < 12; n++)
    {
      if(bitRead(powerpadData, 11 - n) == LOW && bitRead(prevPadData, 11 - n) == HIGH) // Inverted
      {
        // the compressed scheme for NESLCD ROM patches
        uint8_t btn = PowerPadBtnMap[n];
        if (btn == 0 or btn == 6)
        {
          bleGamepad.setHat(HAT_UP); 
          bleGamepad.release(BUTTON_1);
          bleGamepad.release(B_BUTTON);
        }
        else if (btn == 1 or btn == 7)
        {
          bleGamepad.setHat(HAT_RIGHT);
          bleGamepad.release(BUTTON_1);
          bleGamepad.release(B_BUTTON);
        }
        else if (btn == 2 or btn == 8)
        {
          bleGamepad.setHat(HAT_DOWN);
          bleGamepad.release(BUTTON_1);
          bleGamepad.release(B_BUTTON);
        }
        else if (btn == 3 or btn == 9)
        {
          bleGamepad.setHat(HAT_LEFT);
          bleGamepad.release(BUTTON_1);
          bleGamepad.release(B_BUTTON);
        }
        else if (btn == 4 or btn == 10)
        {
          bleGamepad.press(BUTTON_1);
          bleGamepad.setHat(HAT_CENTERED);
          bleGamepad.release(B_BUTTON);
        }
        else if (btn == 5 or btn == 11)
        {
          bleGamepad.press(B_BUTTON);
          bleGamepad.release(BUTTON_1);
          bleGamepad.setHat(HAT_CENTERED);
        }
        
        if(btn > 5)
        {
          bleGamepad.pressStart();
          pressSelect(false);
        }
        else
        {
          bleGamepad.releaseStart();
          pressSelect(true);
        }

        if(DEBUG)
        {
          Serial.print("# BTN: ");
          Serial.print(btn + 1);
          Serial.print(" ( ");
          Serial.print(12 - n);
          Serial.println(" ) Pressed");
        }
        break;
      }
    }

    bleGamepad.sendReport();
  }
  else //If using uncompressed scheme across all 12 buttons)
  {
    if(powerpadData != prevPadData) // If state changed
    {
      for(int n = 0; n < 12; n++)
      {
        if( ( bitRead(powerpadData, 11 - n) == LOW) && ( bitRead(prevPadData, 11 - n) == HIGH)) // Inverted
          outputDirect(true,PowerPadBtnMap[n]);
        else if( ( bitRead(powerpadData, 11 - n) == HIGH) && ( bitRead(prevPadData, 11 - n) == LOW) ) 
          outputDirect(false,PowerPadBtnMap[n]);
      }

      bleGamepad.sendReport();
    }
  }
}

inline void outputZapper(uint16_t zapperData, uint16_t prevPadData) // Output using zapper data
{    
  const bool Light = bitRead(zapperData,0);
  const bool Trigger = bitRead(zapperData,1);
  const bool prevLight = bitRead(prevPadData,0);
  const bool prevTrigger = bitRead(prevPadData,1);

  static bool prevTriggResetData = false;// Previous state of Zapper trigger reset
  static unsigned long triggerTime = 0;  // Time of last trigger pull
  static unsigned long lightTime = 0;    // Time of last light sense

  if(Light && !prevLight) 
  {
    lightTime = millis();
    DebugOut("Light On");
    outputDirect(false,BUTTON_1);
  }
  else if(!Light && prevLight && (millis() - lightTime > LIGHT_PERIOD)) 
  {
    DebugOut("Light Off");
    outputDirect(true,BUTTON_1);
  }

  if(Trigger && !prevTrigger)
  {
    prevTriggResetData = false;
    triggerTime = millis();
    DebugOut("Trigger On");    
    outputDirect(true,B_BUTTON);
  }
  else if(Trigger && prevTrigger && !prevTriggResetData && (millis() - triggerTime > TRIGGER_PERIOD))
  {
    prevTriggResetData = true;
    DebugOut("Trigger Release");
    outputDirect(false,B_BUTTON);
  }
  else if(!Trigger && prevTrigger && (millis() - triggerTime > TRIGGER_PERIOD))
  {
    prevTriggResetData = false;
    DebugOut("Trigger Off");
    outputDirect(false,B_BUTTON);
  } 

  if(zapperData != prevPadData) // If state changed
    bleGamepad.sendReport();
}

void output(padTypes currentType, uint16_t gamepadData)
{
  static uint16_t prevPadData = 65535;   // Previous state of game/power pad state

  if(currentType == gamePad)
    outputGamepad(gamepadData, prevPadData);
  else if(currentType == powerPad)
    outputPowerpad(gamepadData, prevPadData);
  else if(currentType == zapperPad)
    outputZapper(gamepadData, prevPadData);

  prevPadData = gamepadData;
}
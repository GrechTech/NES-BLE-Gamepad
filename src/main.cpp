#include "main.h"

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type
uint16_t prevPadData = 65535;   // Previous state of game/power pad state
bool prevTriggData = false;     // Previous state of Zapper trigger
bool prevTriggResetData = false;// Previous state of Zapper trigger reset
bool prevLightData = true;      // Previous state of Zapper light sensor
unsigned long triggerTime = 0;  // Time of last trigger pull
unsigned long lightTime = 0;    // Time of last light sense

// SETUP
inline void setupGamepad()
{
  setupShiftReg();
  setupBluetooth();

  currentType = gamePad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Game Pad");
  }
}

inline void setupPowerpad()
{
  setupShiftReg();
  setupBluetooth();

  currentType = powerPad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Power Pad");
  }
}

inline void setupZapper()
{
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up
  setupBluetooth();

  currentType = zapperPad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Zapper Pad");
  }
}


// READ
inline void readGamepad()
{
  uint8_t gamepadData = (uint8_t)readShiftReg(false); // Get state
  if((uint16_t)gamepadData != prevPadData) // If state changed
  {
    outputGamepad(gamepadData, prevPadData);
  }

  prevPadData = (uint16_t)gamepadData;
}

inline void readPowerpad()
{
  // Button map reference https://www.nesdev.org/wiki/Power_Pad

  uint16_t powerpadData = readShiftReg(true); // Get state

  if(compressPowerpad)
  {
    outputPowerpad(powerpadData, prevPadData);
  }
  else //if not compressPowerpad
  {
    if(powerpadData != prevPadData) // If state changed
    {
      for(int n = 0; n < 12; n++)
      {
        if( ( bitRead(powerpadData, 11 - n) == LOW) && ( bitRead(prevPadData, 11 - n) == HIGH)) // Inverted
        {
          outputDirect(true,PowerPadBtnMap[n]);
          if (DEBUG)
          {
            Serial.print("# BTN: ");
            Serial.print(PowerPadBtnMap[n]);
            Serial.println(" Pressed");
          }
        }
        else if( ( bitRead(powerpadData, 11 - n) == HIGH) && ( bitRead(prevPadData, 11 - n) == LOW) ) 
        {
          outputDirect(false,PowerPadBtnMap[n]);
          if (DEBUG)
          {
            Serial.print("# BTN: ");
            Serial.print(PowerPadBtnMap[n]);
            Serial.println(" Released");
          }
        }
      }
    }

    prevPadData = powerpadData;
    updatePad();
  }


}

inline void readZapper()
{
  bool changed = false;

  if(digitalRead(LIGHT_PIN) && !prevLightData) 
  {
    prevLightData = true;
    outputDirect(false,1);
    changed = true;
    lightTime = millis();
    if (DEBUG)
    {
      Serial.println("Light On (Inverted)");
    }
  }
  else if(!digitalRead(LIGHT_PIN) && prevLightData && (millis() - lightTime > lightPeriod)) 
  {
    prevLightData = false;
    outputDirect(true,1);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Light Off (Inverted)");
    }
  }

  if(digitalRead(TRIGG_PIN) && !prevTriggData)
  {
    prevTriggData = true;
    prevTriggResetData = false;
    outputDirect(true,4);
    changed = true;
    triggerTime = millis();
    if (DEBUG)
    {
      Serial.println("Trigger On");
    }
  }
  else if(digitalRead(TRIGG_PIN) && prevTriggData && !prevTriggResetData && (millis() - triggerTime > triggerPeriod))
  {
    prevTriggData = true;
    prevTriggResetData = true;
    outputDirect(false,4);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Trigger Release");
    }
  }
  else if(!digitalRead(TRIGG_PIN) && prevTriggData && (millis() - triggerTime > triggerPeriod))
  {
    prevTriggData = false;
    prevTriggResetData = false;
    outputDirect(false,4);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Trigger Off");
    }
  }

  if (changed)
  {
    updatePad();
    if (DEBUG)
    {
      Serial.print(".");
    }
  }
}

inline padTypes detectType()
{
  bool gamepadIndicator = false;
  bool powerpadIndicator = false;
  bool zapperIndicator = false;

  // Game Pad Check
  pinMode(GAMEPAD_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(GAMEPAD_PIN))
  {
    gamepadIndicator = true;
    if (DEBUG)
    {
      Serial.println("#### Game Pad Indicated");
    }
  }
  pinMode(GAMEPAD_PIN, INPUT);

  // Power Pad Check
  pinMode(POWERPAD1_PIN, INPUT_PULLDOWN);
  pinMode(POWERPAD2_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(POWERPAD1_PIN) && digitalRead(POWERPAD1_PIN))
  {
    powerpadIndicator = true;
    if (DEBUG)
    {
      Serial.println("#### Power Pad Indicated");
    }
  }

  // Zapper Check
  pinMode(TRIGG_PIN, INPUT_PULLUP);
  delay(1);
  if(!digitalRead(TRIGG_PIN))
  {
    zapperIndicator = true;
    if (DEBUG)
    {
      Serial.println("#### Zapper Indicated");
    }
  }  
  pinMode(TRIGG_PIN, INPUT);  

  // Decide
  if(gamepadIndicator) // Game Pad Pin only active with gamepad
  {
    if (DEBUG)
    {
      Serial.println("#### Game Pad Mode");
    }
    return gamePad;
  }  
  else if(!gamepadIndicator && powerpadIndicator && !zapperIndicator)
  {
    if (DEBUG)
    {
      Serial.println("#### Power Pad Mode");
    }
    return powerPad;
  }  
  else if(!gamepadIndicator && !powerpadIndicator && zapperIndicator)
  {
    if (DEBUG)
    {
      Serial.println("#### Zapper Mode");
    }
    return zapperPad;
  }  
  else
  {
    if (DEBUG)
    {
      Serial.println("#### Detection Failed");
    }
    return noPad;
  }  
}

// MAIN
void setup()
{
  padTypes type = forceMode; // Represents the type of NES accessory

  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.println("### Setup Start");
  }

  if(forceMode == noPad)
  {
    if (DEBUG)
    {
      Serial.print("### Auto Detect Start");
    }

    while (type == noPad)
    {
      type = detectType();

      if (DEBUG)
      {
        Serial.print('.');
      }
    } 

    if (DEBUG)
    {
      Serial.println();
      Serial.println("### Auto Detect Complete");
    }
  }

  switch(type)
  {
    case noPad:
      if (DEBUG)
      {
        Serial.println("### No controller detected");
      }
      break;
    case gamePad:
      if (DEBUG)
      {
        Serial.println("### Start Setup Game Pad");
      }  
      setupGamepad();
      break;
    case powerPad:
      if (DEBUG)
      {
        Serial.println("### Start Setup Power Pad");
      }
      setupPowerpad();
      break;
    case zapperPad:
      if (DEBUG)
      {
        Serial.println("### Start Setup Zapper");
      }
      setupZapper();
      break;
  }

  if (DEBUG)
  {
    Serial.println("### Setup Done");
  }
}

void loop()
{
  if (connected())
  {
    switch(currentType)
    {
      case noPad:
        if (DEBUG_ADV)
        {
          Serial.println("ERROR: NO CONTROLLER");
        }
        break;
      case gamePad:
        readGamepad();
        break;
      case powerPad:
        readPowerpad();
        break;
      case zapperPad:
        readZapper();
        break;
    }
  }

  if(DEBUG_ADV)
  {
    delay(1000);
  }
  else if(currentType == powerPad && compressPowerpad)
  {
    delay(16);
  }
  else
  {
    delay(2);
  }
}
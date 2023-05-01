#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

//-- CONFIG --//
const bool DEBUG = false; // Enable for serial monitor priority debug outputs
const bool DEBUG_ADV = false; // Enable for serial monitor advanced debug outputs

// ENUMERATIONS
enum padTypes // Defines the supported types of NES controller input
{   
  noPad = 0,      //  No known pad detected successfully (Includes no connection)
  gamePad = 1,    //  Normal NES controller detected
  powerPad = 2,   //  NES Power Pad detected
  zapperPad = 3   //  NES Zapper detected (Only tested with Tomee Zapp - 1st May 2023)
};

enum pins // ESP32 Pin Labels
{   
  GAMEPAD_PIN = 18,     //  PowerPad or Normal Pad only
  POWERPAD2_PIN = 19,   //  Power Pad only (= TRIGG_PIN)
  POWERPAD1_PIN = 21,   //  Power Pad only (= LIGHT_PIN)

  TRIGG_PIN = 19,     //  Zapper only (= PP_OUT2_PIN)
  LIGHT_PIN = 21,       //  Zapper only (= PP_OUT1_PIN)

  CLOCK_PIN = 22,       //  PowerPad or Normal Pad only
  LATCH_PIN = 23,       //  PowerPad or Normal Pad only
};

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type

uint16_t prevPadData = 65535;     // Previous state of game/power pad state
bool prevTriggData = false;     // Previous state of Zapper trigger
bool prevLightData = true;      // Previous state of Zapper light sensor

// DYNAMIC
BleGamepad bleGamepad("NES Controller", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Sstore all of the Bluetooth options

// SETUP BASE FUNCTIONS
inline void setupBluetooth() // Setup the Bluetooth gamepad service
{
  bleGamepadConfig.setHatSwitchCount(1);    // Set a single D-Pad
  bleGamepadConfig.setIncludeZAxis(false);  // Simplify the HID report 
  bleGamepadConfig.setIncludeRxAxis(false); // By removing unused axis
  bleGamepadConfig.setIncludeRyAxis(false);
  bleGamepadConfig.setIncludeRzAxis(false);
  bleGamepadConfig.setIncludeSlider1(false);
  bleGamepadConfig.setIncludeSlider2(false);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);

  if (DEBUG)
  {
    Serial.println("##### Done Setup BLE");
  }
}

inline void setupShiftReg() // Setup pins to read 4021 shift register(s)
{
  pinMode(LATCH_PIN,OUTPUT);
  pinMode(CLOCK_PIN,OUTPUT);
  
  pinMode(GAMEPAD_PIN,INPUT);
  pinMode(POWERPAD1_PIN,INPUT);
  pinMode(POWERPAD2_PIN,INPUT);
  
  digitalWrite(LATCH_PIN,HIGH);
  digitalWrite(CLOCK_PIN,HIGH);

  if (DEBUG)
  {
    Serial.println("##### Done Setup Latch");
  }
}


// SETUP FINAL FUNCTIONS
inline void setupGamepad()
{
  bleGamepad.deviceName = "NES Game Pad";
  setupShiftReg();
  bleGamepadConfig.setButtonCount(2); 
  setupBluetooth();

  currentType = gamePad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Game Pad");
  }
}

inline void setupPowerpad()
{
  bleGamepad.deviceName = "NES Power Pad";
  setupShiftReg();
  bleGamepadConfig.setButtonCount(12); 
  setupBluetooth();

  currentType = powerPad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Power Pad");
  }
}

inline void setupZapper()
{
  bleGamepad.deviceName = "NES Zapper";
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up

  bleGamepadConfig.setButtonCount(2); 
  setupBluetooth();

  currentType = zapperPad;

  if (DEBUG)
  {
    Serial.println("#### Done Setup Zapper Pad");
  }
}

// READ FUNCTIONS
inline uint16_t readShiftReg(bool powerpad = false) 
{
  // Setup local registers for each input
  uint8_t gamepadData = 0; 
  uint8_t powerpadData1 = 0;
  uint8_t powerpadData2 = 0;

  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(LATCH_PIN, LOW);
  
  // Read 1st bits of each input
  gamepadData = digitalRead(GAMEPAD_PIN);
  powerpadData1 = digitalRead(POWERPAD1_PIN);
  powerpadData2 = digitalRead(POWERPAD2_PIN);
  
  // Read 2nd to 8th bits of each input
  for (int c = 1; c < 8; c++) 
  {
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(2);
    
    if(powerpad)
    {
      powerpadData1 = powerpadData1 << 1;
      powerpadData2 = powerpadData2 << 1;
    
      powerpadData1 = powerpadData1 + digitalRead(POWERPAD1_PIN);
      powerpadData2 = powerpadData2 + digitalRead(POWERPAD2_PIN); 
    }
    else
    {
      gamepadData = gamepadData << 1;
      gamepadData = gamepadData + digitalRead(GAMEPAD_PIN);
    }

    delayMicroseconds(4);
    digitalWrite(CLOCK_PIN, LOW);

    if (DEBUG_ADV)
    {
      Serial.print(c);
    }
  }

  if (DEBUG_ADV)
  {
    Serial.println();
    Serial.print("Game Pad Data: ");
    Serial.println(gamepadData, BIN);
    Serial.print("Power Pad Data: ");
    Serial.print(powerpadData1, BIN);
    Serial.print(" ");
    Serial.println(powerpadData2, BIN);
  }

  if(powerpad)
  {
    // Combine two (8-bit unsigned) bytes into one 16-bit unsigned integer
    uint16_t powerpadData = (256U * powerpadData2) + powerpadData1;
    if (DEBUG_ADV)
    {
      Serial.print("PP: ");
      Serial.println(powerpadData, BIN);
    }
    return powerpadData;
  }
  else
  {
    if (DEBUG_ADV)
    {
      Serial.print("GP: ");
      Serial.println((uint16_t)gamepadData, BIN);
    }
    return (uint16_t)gamepadData;
  }
}

inline void readGamepad()
{
  uint8_t gamepadData = (uint8_t)readShiftReg(); // Get state

  if((uint16_t)gamepadData != prevPadData) // If state changed
  {
    if(((gamepadData & 0) == 0) && ((prevPadData & 0) == 1))  // Inverted
    {
      bleGamepad.press(0);
      if (DEBUG)
        Serial.println("# BTN: A Pressed");
    }
    else if(((gamepadData & 0) == 1) && ((prevPadData & 0) == 0)) 
    {
      bleGamepad.release(0);
      if (DEBUG)
        Serial.println("# BTN: A Released");
    }

    if(((gamepadData & 1) == 0) && ((prevPadData & 1) == 1)) 
    {
      bleGamepad.press(1);
      if (DEBUG)
        Serial.println("# BTN: B Pressed");
    }  
    else if(((gamepadData & 1) == 1) && ((prevPadData & 1) == 0)) 
    {
      bleGamepad.release(1);
      if (DEBUG)
        Serial.println("# BTN: B Released");
    }

    if(((gamepadData & 2) == 0) && ((prevPadData & 2) == 1)) 
    {
      bleGamepad.pressStart();
      if (DEBUG)
        Serial.println("# BTN: Start Pressed");
    }
    else if(((gamepadData & 2) == 1) && ((prevPadData & 2) == 0)) 
    {
      bleGamepad.releaseStart();
      if (DEBUG)
        Serial.println("# BTN: Start Released");
    }

    if(((gamepadData & 3) == 0) && ((prevPadData & 3) == 1))
    {
      bleGamepad.pressSelect();
      if (DEBUG)
        Serial.println("# BTN: Select Pressed");
    }
    else if(((gamepadData & 3) == 1) && ((prevPadData & 3) == 0))
    {
      bleGamepad.releaseSelect();
      if (DEBUG)
        Serial.println("# BTN: Select Released");
    }

    // Check if DPad changed
    if( ((gamepadData & 4) == (prevPadData & 4)) || ((gamepadData & 5) == (prevPadData & 5))
     || ((gamepadData & 6) == (prevPadData & 6)) || ((gamepadData & 7) == (prevPadData & 7))) 
    {
      if (DEBUG)
      Serial.print("# DPAD: ");

      if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 1)
      && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
      {
        bleGamepad.setHat1(1); // UP          (N)
        if (DEBUG)
          Serial.println("1 U");
      }
      else if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 0)
            && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
      {
        bleGamepad.setHat1(2); // UP RIGHT    (NE)
        if (DEBUG)
          Serial.println("2 UR");
      }
      else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 0)
            && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
      {
        bleGamepad.setHat1(3); // RIGHT       (E)
        if (DEBUG)
          Serial.println("3 R");
      }
      else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 0)
            && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 1)) 
      {
        bleGamepad.setHat1(4); // DOWN RIGHT  (SE)
        if (DEBUG)
          Serial.println("# DPAD: 4 DR");
      }
      else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1)
            && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 1)) 
      {
        bleGamepad.setHat1(5); // DOWN        (S)
        if (DEBUG)
          Serial.println("# DPAD: 5 D");
      }
      else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1)
            && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 0)) 
      {
        bleGamepad.setHat1(6);  // DOWN LEFT  (SW)
        if (DEBUG)
          Serial.println("# DPAD: 6 DL");
      }
      else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1)
            && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 0)) 
      {
        bleGamepad.setHat1(7); // LEFT        (W)
        if (DEBUG)
          Serial.println("# DPAD: 7 L");
      }
      else if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 1)
            && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 0)) 
      {
        bleGamepad.setHat1(8); // UP LEFT     (NW)
        if (DEBUG)
          Serial.println("# DPAD: 8 UL");
      }
      else
      {
        bleGamepad.setHat1(0); // CENTRE      (C)
        if (DEBUG)
          Serial.println("0 C");
      }
    }
    

    bleGamepad.sendReport();
  }

  prevPadData = (uint16_t)gamepadData;
}

inline void readPowerpad()
{
  uint16_t powerpadData = readShiftReg(); // Get state

  if(powerpadData != prevPadData) // If state changed
  {
    for(int n = 0; n < 16; n++)
    {
      if( ( (powerpadData & n) == 0) && ( (prevPadData & n) == 1)) // Inverted
      {
        bleGamepad.press(n);
        if (DEBUG)
        {
          Serial.print("# BTN: ");
          Serial.print(n);
          Serial.println(" Pressed");
        }
      }
      else if( ( (powerpadData & n) == 1) && ( (prevPadData & n) == 0) ) 
      {
        bleGamepad.release(n);
        if (DEBUG)
        {
          Serial.print("# BTN: ");
          Serial.print(n);
          Serial.println(" Released");
        }
      }
    }

    bleGamepad.sendReport();
  }

  prevPadData = powerpadData;
}

inline void readZapper()
{
  bool changed = false;

  if(digitalRead(LIGHT_PIN) && !prevLightData) 
  {
    prevLightData = true;
    bleGamepad.release(BUTTON_1); //Inverted Light Pin
    changed = true;
    if (DEBUG)
    {
      Serial.println("Light Off");
    }
  }
  else if(!digitalRead(LIGHT_PIN) && prevLightData) 
  {
    prevLightData = false;
    bleGamepad.press(BUTTON_1);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Light On");
    }
  }

  if(digitalRead(TRIGG_PIN) && !prevTriggData)
  {
    prevTriggData = true;
    bleGamepad.press(BUTTON_2);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Trigger On");
    }
  }
  else if(!digitalRead(TRIGG_PIN) && prevTriggData)
  {
    prevTriggData = false;
    bleGamepad.release(BUTTON_2);
    changed = true;
    if (DEBUG)
    {
      Serial.println("Trigger Off");
    }
  }

  if (changed)
  {
    bleGamepad.sendReport();
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
  }
  pinMode(GAMEPAD_PIN, INPUT);

  // Power Pad Check
  pinMode(POWERPAD1_PIN, INPUT_PULLDOWN);
  pinMode(POWERPAD2_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(POWERPAD1_PIN) || digitalRead(POWERPAD1_PIN))
  {
    powerpadIndicator = true;
  }

  // Zapper Check
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGG_PIN, INPUT_PULLUP);
  delay(1);
  if(digitalRead(LIGHT_PIN) || !digitalRead(TRIGG_PIN))
  {
    zapperIndicator = true;
  }  
  pinMode(LIGHT_PIN, INPUT);
  pinMode(TRIGG_PIN, INPUT);  

  
  if(gamepadIndicator && !powerpadIndicator && !zapperIndicator)
  {
    return gamePad;
  }  
  else if(!gamepadIndicator && powerpadIndicator && !zapperIndicator)
  {
    return powerPad;
  }  
  else if(!gamepadIndicator && !powerpadIndicator && zapperIndicator)
  {
    return zapperPad;
  }  
  else
  {
    return noPad;
  }  
}

void setup()
{
  padTypes type = noPad; // Represents the type of NES accessory

  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.println("### Setup Start");
  }

  while (type == 0);
  {
    type = detectType();
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
  if (bleGamepad.isConnected())
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
}
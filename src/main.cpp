#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

// ENUMERATIONS
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

enum padTypes // Defines the supported types of NES controller input
{   
  noPad = 0,      //  No known pad detected successfully (Includes no connection)
  gamePad = 1,    //  Normal NES controller detected
  powerPad = 2,   //  NES Power Pad detected
  zapperPad = 3   //  NES Zapper detected (Only tested with Tomee Zapp - 1st May 2023)
};

//---------- CONFIG ----------//
const padTypes forceMode = noPad; // Force a given pad mode, Auto detect if noPad selected
const bool DEBUG = false; // Enable for serial monitor priority debug outputs
//---------- CONFIG ----------//

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type
uint16_t prevPadData = 65535;   // Previous state of game/power pad state

// DYNAMIC
BleGamepad bleGamepad("NES Controller", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Sstore all of the Bluetooth options

// SETUP BASE FUNCTIONS
inline void setupBluetooth() // Setup the Bluetooth gamepad service
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
  bleGamepadConfig.setIncludeStart(false);
  bleGamepadConfig.setIncludeSelect(false);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);

  if (DEBUG)
    Serial.println("##### Done Setup BLE");
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
    Serial.println("##### Done Setup Latch");
}


// SETUP FINAL FUNCTIONS
inline void setupGamepad()
{
  setupShiftReg();
  setupBluetooth();

  currentType = gamePad;

  if (DEBUG)
    Serial.println("#### Done Setup Game Pad");
}

inline void setupPowerpad()
{
  setupShiftReg();
  setupBluetooth();

  currentType = powerPad;

  if (DEBUG)
    Serial.println("#### Done Setup Power Pad");
}

inline void setupZapper()
{
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up
  setupBluetooth();

  currentType = zapperPad;

  if (DEBUG)
    Serial.println("#### Done Setup Zapper Pad");
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
    delayMicroseconds(2);
  }

  if(powerpad)
  {
    // Combine two (8-bit unsigned) bytes into one 16-bit unsigned integer
    uint16_t powerpadData = ((256U * (uint16_t)powerpadData1) + (uint16_t)powerpadData2) >> 4;
    return powerpadData;
  }
  else
  {
    return (uint16_t)gamepadData;
  }
}

inline void pressStart(bool input) // Press the select button
{
  if(input)
    bleGamepad.press(BUTTON_12);
  else
    bleGamepad.release(BUTTON_12);
}

inline void pressSelect(bool input) // Press the select button
{
  if(input)
    bleGamepad.press(BUTTON_8);
  else
    bleGamepad.release(BUTTON_8);
}

inline void readGamepad()
{
  uint8_t gamepadData = (uint8_t)readShiftReg(); // Get state

  if((uint16_t)gamepadData != prevPadData) // If state changed
  {
    if((bitRead(gamepadData ,  7 - 0) == LOW) && (bitRead(prevPadData ,7 -  0) == HIGH))  // Inverted
    {
      bleGamepad.press(BUTTON_1);
      if (DEBUG)
        Serial.println("# BTN: A Pressed");
    }
    else if((bitRead(gamepadData , 7 - 0) == HIGH) && (bitRead(prevPadData , 7 - 0) == LOW)) 
    {
      bleGamepad.release(BUTTON_1);
      if (DEBUG)
        Serial.println("# BTN: A Released");
    }

    if((bitRead(gamepadData , 7 - 1) == LOW) && (bitRead(prevPadData , 7 - 1) == HIGH)) 
    {
      bleGamepad.press(BUTTON_4);
      if (DEBUG)
        Serial.println("# BTN: B Pressed");
    }  
    else if((bitRead(gamepadData , 7 - 1) == HIGH) && (bitRead(prevPadData , 7 - 1) == LOW)) 
    {
      bleGamepad.release(BUTTON_4);
      if (DEBUG)
        Serial.println("# BTN: B Released");
    }

    if((bitRead(gamepadData , 7 - 2) == LOW) && (bitRead(prevPadData , 7 - 2) == HIGH)) 
    {
      pressSelect(true);
      if (DEBUG)
        Serial.println("# BTN: Select Pressed");
    }
    else if((bitRead(gamepadData , 7 - 2) == HIGH) && (bitRead(prevPadData , 7 - 2) == LOW)) 
    {
      pressSelect(false);
      if (DEBUG)
        Serial.println("# BTN: Select Released");
    }

    if((bitRead(gamepadData , 7 - 3) == LOW) && (bitRead(prevPadData , 7 - 3) == HIGH))
    {
      pressStart(true);
      if (DEBUG)
        Serial.println("# BTN: Start Pressed");
      
    }
    else if((bitRead(gamepadData , 7 - 3) == HIGH) && (bitRead(prevPadData , 7 - 3) == LOW))
    {
      pressStart(false);
      if (DEBUG)
        Serial.println("# BTN: Start Released");
    }

    // Check if DPad changed
    if( (bitRead(gamepadData , 7 - 4) != bitRead(prevPadData , 7 - 4)) 
      || (bitRead(gamepadData , 7 - 5) != bitRead(prevPadData , 7 - 5))
      || (bitRead(gamepadData , 7 - 6) != bitRead(prevPadData , 7 - 6)) 
      || (bitRead(gamepadData , 7 - 7) != bitRead(prevPadData , 7 - 7))) 
    {
      if (DEBUG)
        Serial.print("# DPAD: ");
        

      if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
      && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(1); // UP          (N)
        if (DEBUG)
          Serial.println("1 U");
      }
      else if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(2); // UP RIGHT    (NE)
        if (DEBUG)
          Serial.println("2 UR");
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(3); // RIGHT       (E)
        if (DEBUG)
          Serial.println("3 R");
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(4); // DOWN RIGHT  (SE)
        if (DEBUG)
          Serial.println("4 DR");
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(5); // DOWN        (S)
        if (DEBUG)
          Serial.println("5 D");
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(6);  // DOWN LEFT  (SW)
        if (DEBUG)
          Serial.println("6 DL");
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(7); // LEFT        (W)
        if (DEBUG)
          Serial.println("7 L");
      }
      else if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(8); // UP LEFT     (NW)
        if (DEBUG)
          Serial.println("8 UL");
      }
      else
      {
        bleGamepad.setHat1(0); // CENTRE      (C)
        if (DEBUG)
          Serial.println("0 C");
      }
    }

    bleGamepad.sendReport();

    if (DEBUG)
      Serial.println(gamepadData, BIN);
  }

  prevPadData = (uint16_t)gamepadData;
}

inline uint16_t testPowerpad()
{
  const uint16_t period = 250; // ~ two seconds
  uint16_t outputData = 0;
  static uint16_t prevData = 0;
  static unsigned int outputCount = 0;
  
  if(outputCount > period * 12)
    outputCount = 0;
    
  for(int n = 0; n < 12; n++)
  {
    if(outputCount >  (period * n) && outputCount <= period * (n + 1) )
    {
      outputData = pow(2, n);
      outputData = ~outputData;
    }
  }

  outputCount++;
  prevData = outputData;

  return outputData;
}

inline void readPowerpad()
{
  // Button map reference https://www.nesdev.org/wiki/Power_Pad
  static const uint8_t PowerPadBtnMap [12] = {2, 1, 5, 9, 6, 10, 11, 7, 4, 3, 12, 8};
  static bool toggleDevice = false;
  toggleDevice = !toggleDevice;

  uint16_t powerpadData = readShiftReg(true); // Get state
  //uint16_t powerpadData = testPowerpad(); // Get state Test

  uint8_t offset = 0;

  if(toggleDevice)
  {
    pressStart(true);
    pressSelect(false);
    offset = 2;
  }
  else
  {
    pressStart(false);
    pressSelect(true);
  }

  bleGamepad.release(BUTTON_1);
  bleGamepad.release(BUTTON_4);
  bleGamepad.setHat(HAT_CENTERED); 

  #pragma unroll
  for(int n = 0; n < 12; n++)
  {
    if(bitRead(powerpadData, 11 - n) == LOW) // Inverted
    {
      // the compressed scheme for NESLCD ROM patches
      uint8_t btn = PowerPadBtnMap[n];
      
      if (btn == (1 + offset))
        bleGamepad.setHat(HAT_UP); 
      else if (btn == (2 + offset))
        bleGamepad.setHat(HAT_RIGHT);
      else if (btn == (5 + offset))
        bleGamepad.setHat(HAT_DOWN);
      else if (btn == (6 + offset))
        bleGamepad.setHat(HAT_LEFT);
      else if (btn == (9 + offset))
        bleGamepad.press(BUTTON_1);
      else if (btn == (10 + offset))
        bleGamepad.press(BUTTON_4);

      if(DEBUG)
      {
        Serial.print("# BTN: ");
        Serial.print(btn);
        Serial.print(" ( ");
        Serial.print(12 - n);
        Serial.print(" ) Pressed ");
        Serial.println(toggleDevice);
      }
    }
  }
  bleGamepad.sendReport();

  delay(14);
}

inline void readZapper()
{
  const uint16_t triggerPeriod = 100;   // Trigger debounce & reset time (ms)
  const uint16_t lightPeriod = 20;      // Light sensor input debounce time (ms)
  static bool prevTriggData = false;     // Previous state of Zapper trigger
  static bool prevTriggResetData = false;// Previous state of Zapper trigger reset
  static bool prevLightData = true;      // Previous state of Zapper light sensor
  static unsigned long triggerTime = 0;  // Time of last trigger pull
  static unsigned long lightTime = 0;    // Time of last light sense

  bool changed = false;

  if(digitalRead(LIGHT_PIN) && !prevLightData) 
  {
    prevLightData = true;
    bleGamepad.release(BUTTON_1); //Inverted Light Pin
    changed = true;
    lightTime = millis();
    if (DEBUG)
      Serial.println("Light On (Inverted)");
  }
  else if(!digitalRead(LIGHT_PIN) && prevLightData && (millis() - lightTime > lightPeriod)) 
  {
    prevLightData = false;
    bleGamepad.press(BUTTON_1);
    changed = true;
    if (DEBUG)
      Serial.println("Light Off (Inverted)");
  }

  if(digitalRead(TRIGG_PIN) && !prevTriggData)
  {
    prevTriggData = true;
    prevTriggResetData = false;
    bleGamepad.press(BUTTON_4);
    changed = true;
    triggerTime = millis();
    if (DEBUG)
      Serial.println("Trigger On");
  }
  else if(digitalRead(TRIGG_PIN) && prevTriggData && !prevTriggResetData && (millis() - triggerTime > triggerPeriod))
  {
    prevTriggData = true;
    prevTriggResetData = true;
    bleGamepad.release(BUTTON_4);
    changed = true;
    if (DEBUG)
      Serial.println("Trigger Release");
  }
  else if(!digitalRead(TRIGG_PIN) && prevTriggData && (millis() - triggerTime > triggerPeriod))
  {
    prevTriggData = false;
    prevTriggResetData = false;
    bleGamepad.release(BUTTON_4);
    changed = true;
    if (DEBUG)
      Serial.println("Trigger Off");
  }

  if (changed)
  {
    bleGamepad.sendReport();
    if (DEBUG)
      Serial.print(".");
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
      Serial.println("#### Game Pad Indicated");
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
      Serial.println("#### Power Pad Indicated");
  }

  // Zapper Check
  pinMode(TRIGG_PIN, INPUT_PULLUP);
  delay(1);
  if(!digitalRead(TRIGG_PIN))
  {
    zapperIndicator = true;
    if (DEBUG)
      Serial.println("#### Zapper Indicated");
  }  
  pinMode(TRIGG_PIN, INPUT);  

  // Decide
  if(gamepadIndicator) // Game Pad Pin only active with gamepad
  {
    if (DEBUG)
      Serial.println("#### Game Pad Mode");
    return gamePad;
  }  
  else if(!gamepadIndicator && powerpadIndicator && !zapperIndicator)
  {
    if (DEBUG)
      Serial.println("#### Power Pad Mode");
    return powerPad;
  }  
  else if(!gamepadIndicator && !powerpadIndicator && zapperIndicator)
  {
    if (DEBUG)
      Serial.println("#### Zapper Mode");
    return zapperPad;
  }  
  else
  {
    if (DEBUG)
      Serial.println("#### Detection Failed");
    return noPad;
  }  
}

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
      Serial.println("### Auto Detect Start");

    while (type == noPad)
      type = detectType();

    if (DEBUG)
      Serial.println("### Auto Detect Complete");
  }

  if(type == gamePad)
    setupGamepad();
  else if(type == powerPad)
    setupPowerpad();
  else if(type == zapperPad)
    setupZapper();

  if (DEBUG)
    Serial.println("### Setup Done");
}

void loop()
{
  if (bleGamepad.isConnected())
  {
    if(currentType == gamePad)
      readGamepad();
    else if(currentType == powerPad)
      readPowerpad();
    else if(currentType == zapperPad)
      readZapper();
  }

  delay(2);
}
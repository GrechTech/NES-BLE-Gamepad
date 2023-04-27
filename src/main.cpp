#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

// PINS
const int CLOCK_PIN = 22;    //  PowerPad or Normal Pad only
const int LATCH_PIN = 23;    //  PowerPad or Normal Pad only
const int GAMEPAD_PIN = 18;       //  PowerPad or Normal Pad only

const int LIGHT_PIN = 21;    //  Zapper only (= PP_OUT1_PIN)
const int TRIGGER_PIN = 19;  //  Zapper only (= PP_OUT2_PIN)
const int POWERPAD1_PIN = 21;  //  Power Pad only (= LIGHT_PIN)
const int POWERPAD2_PIN = 19;  //  Power Pad only (= TRIGGER_PIN)

// CONSTANTS
const bool DEBUG = false;

enum padTypes 
{   
  none = 0, 
  gamepad = 1, 
  powerpad = 2,
  zapper = 3
};

// REGISTERS
uint8_t padType = 0;

bool lightData = true;
bool triggerData = false;

uint16_t prevPadData = 0;

bool Changed = false;

// DYNAMIC
BleGamepad bleGamepad("NES Controller", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Create a BleGamepadConfiguration object to store all of the options


// SETUP BASE FUNCTIONS
inline void setupBluetooth()
{
  bleGamepadConfig.setHatSwitchCount(1);
  bleGamepadConfig.setIncludeZAxis(false); // Simplify the HID report 
  bleGamepadConfig.setIncludeRxAxis(false);
  bleGamepadConfig.setIncludeRyAxis(false);
  bleGamepadConfig.setIncludeRzAxis(false);
  bleGamepadConfig.setIncludeSlider1(false);
  bleGamepadConfig.setIncludeSlider2(false);
  
  bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

  bleGamepad.begin(&bleGamepadConfig);
}

inline void setupLatch()
{
  pinMode(LATCH_PIN,OUTPUT);
  pinMode(CLOCK_PIN,OUTPUT);
  
  pinMode(GAMEPAD_PIN,INPUT);
  pinMode(POWERPAD1_PIN,INPUT);
  pinMode(POWERPAD2_PIN,INPUT);
  
  digitalWrite(LATCH_PIN,HIGH);
  digitalWrite(CLOCK_PIN,HIGH);
}


// SETUP FINAL FUNCTIONS
inline void setupGamepad()
{
  bleGamepad.deviceName = "NES Game Pad";
  setupLatch();
  bleGamepadConfig.setButtonCount(2); 
  setupBluetooth();
}

inline void setupPowerpad()
{
  bleGamepad.deviceName = "NES Power Pad";
  setupLatch();
  bleGamepadConfig.setButtonCount(12); 
  setupBluetooth();
}

inline void setupZapper()
{
  bleGamepad.deviceName = "NES Zapper";
  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
  // When trigger pulled, switch disconnected from GND allowing it to be pulled up

  bleGamepadConfig.setButtonCount(2); 
  setupBluetooth();

  bleGamepad.begin(&bleGamepadConfig);
}

// READ FUNCTIONS
inline uint16_t readLatch(bool powerpad = false) 
{
  uint8_t gamepadData = 0;
  uint8_t powerpadData1 = 0;
  uint8_t powerpadData2 = 0;

  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(LATCH_PIN, LOW);
  
  gamepadData = digitalRead(GAMEPAD_PIN);
  powerpadData1 = digitalRead(POWERPAD1_PIN);
  powerpadData2 = digitalRead(POWERPAD2_PIN);
    
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

    if (DEBUG)
    {
      Serial.print(c);
    }
  }

  if (DEBUG)
  {
    Serial.println();
    Serial.println(gamepadData, BIN);
    Serial.println(powerpadData1, BIN);
    Serial.println(powerpadData2, BIN);
  }

  if(powerpad)
  {
    uint16_t powerpadData = 256U * powerpadData2 + powerpadData1;
    return powerpadData;
  }
  else
  {
    return (uint16_t)gamepadData;
  }
}

inline void readGamepad()
{
  uint8_t gamepadData = (uint8_t)readLatch();

  if((gamepadData & 0) == 0) 
    bleGamepad.press(0);
  else
    bleGamepad.release(0);

  if((gamepadData & 1) == 0) 
    bleGamepad.press(1);
  else
    bleGamepad.release(1);

  if((gamepadData & 2) == 0) 
    bleGamepad.pressStart();
  else
    bleGamepad.releaseStart();

  if((gamepadData & 3) == 0) 
    bleGamepad.pressSelect();
  else
    bleGamepad.releaseSelect();

  if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 1) && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
    bleGamepad.setHat1(1);
  else if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 0) && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
    bleGamepad.setHat1(2);
  else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 0) && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 1)) 
    bleGamepad.setHat1(3);
  else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 0) && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 1)) 
    bleGamepad.setHat1(4);
  else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1) && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 1)) 
    bleGamepad.setHat1(5);
  else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1) && ((gamepadData & 6) == 0) && ((gamepadData & 7) == 0)) 
    bleGamepad.setHat1(6);
  else if( ((gamepadData & 4) == 1) && ((gamepadData & 5) == 1) && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 0)) 
    bleGamepad.setHat1(7);
  else if( ((gamepadData & 4) == 0) && ((gamepadData & 5) == 1) && ((gamepadData & 6) == 1) && ((gamepadData & 7) == 0)) 
    bleGamepad.setHat1(8);
  else
    bleGamepad.setHat1(0);

  if((uint16_t)gamepadData != prevPadData)
  {
    bleGamepad.sendReport();
  }

  prevPadData = (uint16_t)gamepadData;
}

inline void readPowerpad()
{
  uint16_t powerpadData = readLatch();

  for(int n = 0; n < 16; n++)
  {
    if((powerpadData & 0) == 0) 
      bleGamepad.press(n);
    else
      bleGamepad.release(n);
  }

  if(powerpadData != prevPadData)
  {
    bleGamepad.sendReport();
  }

  prevPadData = powerpadData;
}

inline void readZapper()
{
  Changed = false;

    if(digitalRead(LIGHT_PIN) && !lightData) 
    {
      lightData = true;
      bleGamepad.release(BUTTON_1); //Inverted Light Pin
      Changed = true;
      if (DEBUG)
      {
        Serial.println("Light Off");
      }
    }
    else if(!digitalRead(LIGHT_PIN) && lightData) 
    {
      lightData = false;
      bleGamepad.press(BUTTON_1);
      Changed = true;
      if (DEBUG)
      {
        Serial.println("Light On");
      }
    }

    if(digitalRead(TRIGGER_PIN) && !triggerData)
    {
      triggerData = true;
      bleGamepad.press(BUTTON_2);
      Changed = true;
      if (DEBUG)
      {
        Serial.println("Trigger On");
      }
    }
    else if(!digitalRead(TRIGGER_PIN) && triggerData)
    {
      triggerData = false;
      bleGamepad.release(BUTTON_2);
      Changed = true;
      if (DEBUG)
      {
        Serial.println("Trigger Off");
      }
    }

    if (Changed)
    {
      bleGamepad.sendReport();
      if (DEBUG)
      {
        Serial.print(".");
      }
    }
}

inline uint8_t detectType()
{
  bool gamepad = false;
  bool powerpad = false;
  bool zapper = false;

  pinMode(GAMEPAD_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(GAMEPAD_PIN))
    gamepad = true;
  pinMode(GAMEPAD_PIN, INPUT);

  pinMode(POWERPAD1_PIN, INPUT_PULLDOWN);
  pinMode(POWERPAD2_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(POWERPAD1_PIN) || digitalRead(POWERPAD1_PIN))
    powerpad = true;

  pinMode(LIGHT_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  delay(1);
  if(digitalRead(LIGHT_PIN) || !digitalRead(TRIGGER_PIN))
    zapper = true;
  pinMode(LIGHT_PIN, INPUT);
  pinMode(TRIGGER_PIN, INPUT);  

  
  if(gamepad && !powerpad && !zapper)
    return 1;
  else if(!gamepad && powerpad && !zapper)
    return 2;
  else if(!gamepad && !powerpad && zapper)
    return 3;
  else
    return 0;
}

void setup()
{
  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.print("Start");
  }

  uint8_t type = 0;

  while (type == 0);
  {
    type = detectType();
  } 
  
  padType = type;

  switch(padType)
  {
    case none:
      if (DEBUG)
        Serial.print("No controller detected");
      break;
    case gamepad:
      setupGamepad();
    case powerpad:
      setupPowerpad();
    case zapper:
      setupZapper();
  }
}

void loop()
{
  if (bleGamepad.isConnected())
  {
    switch(padType)
    {
      case none:
        break;
      case gamepad:
        readGamepad();
        break;
      case powerpad:
        readPowerpad();
        break;
      case zapper:
        readZapper();
        break;
    }
  }
}
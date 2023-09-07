#include "main.h"

// Detect what is connected to the controller port, supports gamepad, powerpad or zapper
padTypes detectType()
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
    DebugOut("#### Game Pad Indicated");
  }
  DebugOut("Gamepad pin (18) state (Pulled down): " + digitalRead(GAMEPAD_PIN));
  pinMode(GAMEPAD_PIN, INPUT);

  // Power Pad Check
  pinMode(POWERPAD1_PIN, INPUT_PULLDOWN);
  pinMode(POWERPAD2_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(POWERPAD1_PIN) || digitalRead(POWERPAD2_PIN))
  {
    powerpadIndicator = true;
    DebugOut("#### Power Pad Indicated");
  }
  if(DEBUG)
     Serial.println("Powerpad pin (21 & 19) states (Pulled up): " + digitalRead(POWERPAD1_PIN) + digitalRead(POWERPAD2_PIN));
  // Zapper Check
  pinMode(TRIGG_PIN, INPUT_PULLUP);
  delay(1);
  if(!digitalRead(TRIGG_PIN))
  {
    zapperIndicator = true;
    DebugOut("#### Zapper Indicated");
  }  
  DebugOut("Gamepad pin (19) state (Pulled down - Seeking inverse): " + digitalRead(TRIGG_PIN));
  pinMode(TRIGG_PIN, INPUT);  

  if(DEBUG)
  Serial.println("Gamepad, powerpad, zapper indicators: " + gamepadIndicator + powerpadIndicator + zapperIndicator);
  // Decide
  if(gamepadIndicator) // Game Pad Pin only active with gamepad
  {
    DebugOut("#### Game Pad Mode");
    
    return gamePad;
  }  
  else if(!gamepadIndicator && powerpadIndicator && !zapperIndicator)
  {
    DebugOut("#### Power Pad Mode");

    return powerPad;
  }  
  else if(!gamepadIndicator && !powerpadIndicator && zapperIndicator)
  {
    DebugOut("#### Zapper Mode");
    
    return zapperPad;
  }  
  else
  {
    DebugOut("#### Detection Failed");
    
    return noPad;
  }  
}

void setupShiftReg() // Setup pins to read 4021 shift register(s)
{
  pinMode(LATCH_PIN,OUTPUT);
  pinMode(CLOCK_PIN,OUTPUT);
  
  pinMode(GAMEPAD_PIN,INPUT);
  pinMode(POWERPAD1_PIN,INPUT);
  pinMode(POWERPAD2_PIN,INPUT);
  
  digitalWrite(LATCH_PIN,HIGH);
  digitalWrite(CLOCK_PIN,HIGH);

  DebugOut("##### Done Setup Latch");
}

inline uint16_t readShiftReg(bool powerpad) // read 4021 shift register(s)
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
    // Combine two (8-bit unsigned) bytes into one 16-bit unsigned integer
    return ((256U * (uint16_t)powerpadData1) + (uint16_t)powerpadData2) >> 4; 
  else
    return (uint16_t)gamepadData;
}

uint16_t input(padTypes currentType)
{
  if(OUTPUT_TEST)
    return testPowerpad();
  else if(currentType == gamePad)
    return readShiftReg(false); // Get state
  else if(currentType == powerPad)
    return readShiftReg(true); // Get state
  else
    return 0;
}
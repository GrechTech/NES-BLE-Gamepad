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
    if (DEBUG)
      Serial.println("#### Game Pad Indicated");
  }
  if(DEBUG)
     Serial.println("Gamepad pin (18) state (Pulled down): " + digitalRead(GAMEPAD_PIN));
  pinMode(GAMEPAD_PIN, INPUT);

  // Power Pad Check
  pinMode(POWERPAD1_PIN, INPUT_PULLDOWN);
  pinMode(POWERPAD2_PIN, INPUT_PULLDOWN);
  delay(1);
  if(digitalRead(POWERPAD1_PIN) || digitalRead(POWERPAD2_PIN))
  {
    powerpadIndicator = true;
    if (DEBUG)
      Serial.println("#### Power Pad Indicated");
  }
  if(DEBUG)
     Serial.println("Powerpad pin (21 & 19) states (Pulled up): " + digitalRead(POWERPAD1_PIN) + digitalRead(POWERPAD2_PIN));
  // Zapper Check
  pinMode(TRIGG_PIN, INPUT_PULLUP);
  delay(1);
  if(!digitalRead(TRIGG_PIN))
  {
    zapperIndicator = true;
    if (DEBUG)
      Serial.println("#### Zapper Indicated");
  }  
  if(DEBUG)
     Serial.println("Gamepad pin (19) state (Pulled down - Seeking inverse): " + digitalRead(TRIGG_PIN));
  pinMode(TRIGG_PIN, INPUT);  

  if(DEBUG)
  Serial.println("Gamepad, powerpad, zapper indicators: " + gamepadIndicator + powerpadIndicator + zapperIndicator);
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

void setupShiftReg() // Setup pins to read 4021 shift register(s)
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
    return ((256U * (uint16_t)powerpadData1) + (uint16_t)powerpadData2) >> 4; 
    // Combine two (8-bit unsigned) bytes into one 16-bit unsigned integer
  else
    return (uint16_t)gamepadData;
}

inline uint16_t readZapper() // Read the zapper light and trigger pins
{
  static bool prevTriggData = false;     // Previous state of Zapper trigger
  static bool prevTriggResetData = false;// Previous state of Zapper trigger reset
  static bool prevLightData = true;      // Previous state of Zapper light sensor
  static unsigned long triggerTime = 0;  // Time of last trigger pull
  static unsigned long lightTime = 0;    // Time of last light sense

  bool changed = false;
  uint16_t data = 0;

  if(digitalRead(LIGHT_PIN) && !prevLightData) 
  {
    prevLightData = true;
    changed = true;
    lightTime = millis();
    if (DEBUG)
      Serial.println("Light On (Inverted)");
    
    data = 1;
  }
  else if(!digitalRead(LIGHT_PIN) && prevLightData && (millis() - lightTime > LIGHT_PERIOD)) 
  {
    prevLightData = false;
    changed = true;
    if (DEBUG)
      Serial.println("Light Off (Inverted)");
  }

  if(digitalRead(TRIGG_PIN) && !prevTriggData)
  {
    prevTriggData = true;
    prevTriggResetData = false;
    changed = true;
    triggerTime = millis();
    if (DEBUG)
      Serial.println("Trigger On");    

    data += 2;
  }
  else if(digitalRead(TRIGG_PIN) && prevTriggData && !prevTriggResetData && (millis() - triggerTime > TRIGGER_PERIOD))
  {
    prevTriggData = true;
    prevTriggResetData = true;
    changed = true;
    if (DEBUG)
      Serial.println("Trigger Release");
  }
  else if(!digitalRead(TRIGG_PIN) && prevTriggData && (millis() - triggerTime > TRIGGER_PERIOD))
  {
    prevTriggData = false;
    prevTriggResetData = false;
    changed = true;
    if (DEBUG)
      Serial.println("Trigger Off");
  }

  return data;
}

uint16_t input(padTypes currentType)
{
  if(OUTPUT_TEST)
    return testPowerpad();
  else if(currentType == gamePad)
    return readShiftReg(false); // Get state
  else if(currentType == powerPad)
    return readShiftReg(true); // Get state
  else if(currentType == zapperPad)
    return readZapper();
  else
    return 0;
}
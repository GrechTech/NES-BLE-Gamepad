#include "main.h"

bool prevTriggData = false;     // Previous state of Zapper trigger
bool prevTriggResetData = false;// Previous state of Zapper trigger reset
bool prevLightData = true;      // Previous state of Zapper light sensor
unsigned long triggerTime = 0;  // Time of last trigger pull
unsigned long lightTime = 0;    // Time of last light sense

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
  {
    Serial.println("##### Done Setup Latch");
  }
}

uint16_t readShiftReg(bool powerpad) // read 4021 shift register(s)
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
    uint16_t powerpadData = ((256U * (uint16_t)powerpadData1) + (uint16_t)powerpadData2) >> 4;
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

uint16_t readZapper()
{
  bool changed = false;
  uint16_t data = 0;

  if(digitalRead(LIGHT_PIN) && !prevLightData) 
  {
    prevLightData = true;
    changed = true;
    lightTime = millis();
    if (DEBUG)
    {
      Serial.println("Light On (Inverted)");
    }
    data = 1;
  }
  else if(!digitalRead(LIGHT_PIN) && prevLightData && (millis() - lightTime > lightPeriod)) 
  {
    prevLightData = false;
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
    data += 2;
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

  return data;
}
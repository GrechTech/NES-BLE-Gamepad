#include "main.h"
#include <TaskScheduler.h>

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type
volatile uint16_t gamepadData = 65535;   // Previous state of game/power pad state
volatile uint16_t prevPadData = 65535;   // Previous state of game/power pad state

// Task prototypes
void inputLoop();
void outputLoop();


Scheduler ts; //Task Scheduluer

//Tasks
Task tIn  ( 2 * TASK_MILLISECOND, TASK_FOREVER , &inputLoop,  &ts, true );
Task tOut ( 2 * TASK_MILLISECOND, TASK_FOREVER , &outputLoop, &ts, true );

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
      Serial.print("### Auto Detect Start");

    while (type == noPad)
    {
      type = detectType();

      if (DEBUG)
        Serial.print('.');
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
        Serial.println("### No controller detected");
      break;
    case gamePad: // Setup gamepad
      if (DEBUG)
        Serial.println("### Start Setup Game Pad");
      setupShiftReg();
      setupBluetooth();
      tOut.setInterval(1 * TASK_MILLISECOND);

      currentType = gamePad;

      if (DEBUG)
        Serial.println("#### Done Setup Game Pad");
      break;
    case powerPad: // Setup powerpad
      if (DEBUG)
        Serial.println("### Start Setup Power Pad");
      setupShiftReg();
      setupBluetooth();
      tOut.setInterval(16 * TASK_MILLISECOND);

      currentType = powerPad;

      if (DEBUG)
        Serial.println("#### Done Setup Power Pad");
      break;
    case zapperPad: // Setup zapper
      if (DEBUG)
        Serial.println("### Start Setup Zapper");
      pinMode(LIGHT_PIN, INPUT_PULLUP);
      pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
      // When trigger pulled, switch disconnected from GND allowing it to be pulled up
      setupBluetooth();
      tOut.setInterval(1 * TASK_MILLISECOND);

      currentType = zapperPad;

      if (DEBUG)
        Serial.println("#### Done Setup Zapper Pad");
      break;
  }

  if (DEBUG)
    Serial.println("### Setup Done");
}

void inputLoop()
{
  if (connected())
  {
    if(currentType == gamePad)
      gamepadData = readShiftReg(false); // Get state
    else if(currentType == powerPad)
      gamepadData = readShiftReg(true); // Get state
    else if(currentType == zapperPad)
      gamepadData = readZapper();
  }
}

void outputLoop()
{
  if (connected())
  {
    if(currentType == gamePad)
      outputGamepad(gamepadData, prevPadData);
    else if(currentType == powerPad)
      outputPowerpad(gamepadData, prevPadData, compressPowerpad);
    else if(currentType == zapperPad)
      outputZapper(gamepadData, prevPadData);

    if(gamepadData != prevPadData) // If state changed
      prevPadData = gamepadData;
  }
}

void loop()
{
  ts.execute();
}

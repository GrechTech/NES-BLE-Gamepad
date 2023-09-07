#include "main.h"
#include <TaskScheduler.h>

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type

// MAIN
void setup()
{
  currentType = FORCE_MODE; // Represents the type of NES accessory

  if (DEBUG)
    Serial.begin(115200);
  DebugOut("### Setup Start");

  if(OUTPUT_TEST)
    currentType = powerPad;
  else if(FORCE_MODE == noPad)
  {
    DebugOut("### Auto Detect Start");

    while (currentType == noPad)
    {
      currentType = detectType();
      DebugOut(".", false);
    } 

    DebugOut("### Auto Detect Complete");
  }

  switch(currentType)
  {
    case gamePad: // Setup gamepad
      DebugOut("### Start Setup Game Pad");
      setupShiftReg();
      DebugOut("#### Done Setup Game Pad");
      break;

    case powerPad: // Setup powerpad
      DebugOut("### Start Setup Power Pad");
      setupShiftReg();
      DebugOut("#### Done Setup Power Pad");
      break;

    case zapperPad: // Setup zapper
      DebugOut("### Start Setup Zapper");
      pinMode(LIGHT_PIN, INPUT_PULLUP);
      pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
      // When trigger pulled, switch disconnected from GND allowing it to be pulled up
      
      DebugOut("#### Done Setup Zapper Pad");
      break;
  }

  setupBluetooth();
  DebugOut("### Setup Done");

  delay(1);
}

void loop()
{
  if(connected())
    output(currentType, input(currentType));
}

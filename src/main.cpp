#include "main.h"
#include <TaskScheduler.h>

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type
volatile uint16_t gamepadData = 65535;   // Current state of game/power pad state
bool Connected = false;

// Task prototypes
void inputLoop(); // Inputs loop (Read pins)
void outputLoop(); // Outputs loop (Update BT)
void connectLoop(); // Outputs loop (Update BT)

Scheduler ts; //Task Scheduluer

//Tasks
Task tCon ( 1 * TASK_SECOND,      TASK_FOREVER, &connectLoop,   &ts, false); // Connections check
Task tIn  ( 1 * TASK_MILLISECOND, TASK_FOREVER, &inputLoop,     &ts, Connected); // Inputs
Task tOut ( 1 * TASK_MILLISECOND, TASK_FOREVER, &outputLoop,    &ts, Connected); // Outputs

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
      if(COMPRESS_POWERPAD)
        tOut.setInterval(COMPRESS_SPEED * TASK_MILLISECOND); // 60 Hz
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
  tCon.enable();
  tIn.enable(); // Start output loop
  tOut.enable(); // Start output loop
}

// Main
void connectLoop()
{
  Connected = connected();
}

void inputLoop()
{
  gamepadData = input(currentType);
}

void outputLoop()
{
  output(currentType, gamepadData);
}

void loop()
{
  ts.execute();
}

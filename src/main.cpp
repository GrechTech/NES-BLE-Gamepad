#include "main.h"
#include <TaskScheduler.h>

// REGISTERS
padTypes currentType = noPad; // Stores the current pad type
volatile uint16_t gamepadData = 65535;   // Current state of game/power pad state

// Task prototypes
void inputLoop(); // Inputs loop (Read pins)
void outputLoop(); // Outputs loop (Update BT)

Scheduler ts; //Task Scheduluer

//Tasks
Task tIn  ( 2 * TASK_MILLISECOND, TASK_FOREVER , &inputLoop,  &ts, true ); // Inputs
Task tOut ( 2 * TASK_MILLISECOND, TASK_FOREVER , &outputLoop, &ts, false ); // Outputs

// MAIN
void setup()
{
  currentType = FORCE_MODE; // Represents the type of NES accessory

  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.println("### Setup Start");
  }

  if(OUTPUT_TEST)
    currentType = powerPad;
  else if(FORCE_MODE == noPad)
  {
    if (DEBUG)
      Serial.print("### Auto Detect Start");

    while (currentType == noPad)
    {
      currentType = detectType();

      if (DEBUG)
        Serial.print('.');
    } 

    if (DEBUG)
    {
      Serial.println();
      Serial.println("### Auto Detect Complete");
    }
  }

  switch(currentType)
  {
    case gamePad: // Setup gamepad
      if (DEBUG)
        Serial.println("### Start Setup Game Pad");
      setupShiftReg();
      tOut.setInterval(2 * TASK_MILLISECOND); // 500 Hz
      if (DEBUG)
        Serial.println("#### Done Setup Game Pad");
      break;

    case powerPad: // Setup powerpad
      if (DEBUG)
        Serial.println("### Start Setup Power Pad");
      setupShiftReg();
      if(COMPRESS_POWERPAD)
        tOut.setInterval(COMPRESS_SPEED * TASK_MILLISECOND); // 60 Hz
      else
        tOut.setInterval(16 * TASK_MILLISECOND); // 60 Hz
      if (DEBUG)
        Serial.println("#### Done Setup Power Pad");
      break;

    case zapperPad: // Setup zapper
      if (DEBUG)
        Serial.println("### Start Setup Zapper");
      pinMode(LIGHT_PIN, INPUT_PULLUP);
      pinMode(TRIGG_PIN, INPUT_PULLUP); // Tomee Zapp has a simple switch NC to GND. 
      // When trigger pulled, switch disconnected from GND allowing it to be pulled up
      
      tOut.setInterval(2 * TASK_MILLISECOND); // 500 Hz
      if (DEBUG)
        Serial.println("#### Done Setup Zapper Pad");
      break;
  }

  setupBluetooth();
  tOut.enable(); // Start output loop

  if (DEBUG)
    Serial.println("### Setup Done");
}

// Main
void inputLoop()
{
  if (connected())
    gamepadData = input(currentType);
}

void outputLoop()
{
  if (connected())
    output(currentType, gamepadData);
}

void loop()
{
  ts.execute();
}

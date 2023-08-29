#ifndef main_h
#define main_h

#include <Arduino.h>

enum padTypes // Defines the supported types of NES controller input
{   
  noPad = 0,      //  No known pad detected successfully (Includes no connection)
  gamePad = 1,    //  Normal NES controller detected
  powerPad = 2,   //  NES Power Pad detected
  zapperPad = 3   //  NES Zapper detected (Only tested with Tomee Zapp - 1st May 2023)
};

//---------- CONFIG ----------//
const padTypes forceMode = noPad; // Force a given pad mode, Auto detect if noPad selected
const bool compressPowerpad = false; // Compress the powerpad buttons into the NES gamepad buttons, for use with custom ROMs
const bool emulatorMapping = false; // Remap Select for use with on cosnole emulators
const bool DEBUG = false; // Enable for serial monitor priority debug outputs
const bool DEBUG_ADV = false; // Enable for serial monitor advanced debug outputs
const bool OUTPUT_TEST = false; // Enable for serial monitor advanced debug outputs
const uint16_t triggerPeriod = 100;   // Trigger debounce & reset time (ms)
const uint16_t lightPeriod = 20;      // Light sensor input debounce time (ms)

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

// PROTOTYPES
// INPUT
void setupShiftReg(void);
padTypes detectType(void);
uint16_t readShiftReg(bool powerpad);
uint16_t readZapper(void);

// OUTPUT
void setupBluetooth(void);
bool connected(void);
void outputPowerpad(uint16_t powerpadData, uint16_t prevPadData);
void outputGamepad(uint8_t gamepadData, uint8_t prevPadData);
void outputZapper(uint8_t zapperData, uint8_t prevPadData);

// TEST
uint16_t TestSequence(void);
#endif
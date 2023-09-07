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
const padTypes FORCE_MODE = noPad; // Force a given pad mode, Auto detect if noPad selected
const uint16_t TRIGGER_PERIOD = 100;   // Trigger debounce & reset time (ms)
const uint16_t LIGHT_PERIOD = 20;      // Light sensor input debounce time (ms)
const uint8_t COMPRESS_SPEED = 16; // (ms) How often the powerpad output loop cycles in compression mode
const uint8_t B_BUTTON_EMU_OFFSET = 2; // BUTTON_4 in emulator mode
const bool COMPRESS_POWERPAD = true; // Compress the powerpad buttons into the NES gamepad buttons, for use with custom ROMs
const bool EMULATOR_MAPPING = true; // Remap Select for use with on cosnole emulators
// DEBUG CONFIG
const bool DEBUG = true; // Enable for serial monitor priority debug outputs
const bool OUTPUT_TEST = false; // Enable for serial monitor advanced debug outputs

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

// Debug macro
inline void DebugOut(const char* msg,const bool ln = true)
{
  if(ln)
    Serial.println(msg);
  else
    Serial.print(msg);
}

// PROTOTYPES
// INPUT
void setupShiftReg(void);           // Setup pins to read 4021 shift register(s)
padTypes detectType(void);          // Detect what is connected to the controller port
uint16_t input(padTypes currentType); // Return data for a given padType

// OUTPUT
bool connected(void); // Check if Bluetooth connected
void setupBluetooth(void); // Setup the Bluetooth gamepad service
void output(padTypes currentType, uint16_t gamepadData); // Send data for a given padType

// TEST
uint16_t testPowerpad(void); // Press every powerpad button, incrementing every 2 seconds
#endif
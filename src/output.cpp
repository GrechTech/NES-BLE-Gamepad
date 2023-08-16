#include "main.h"
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

// DYNAMIC
BleGamepad bleGamepad("NES Controller", "GrechTech", 100); // Initialise Bluetooth gamepad
BleGamepadConfiguration bleGamepadConfig;     // Sstore all of the Bluetooth options

// SETUP BASE FUNCTIONS
void setupBluetooth() // Setup the Bluetooth gamepad service
{
    bleGamepad.deviceName = "NES BLE Gamepad";
    bleGamepadConfig.setHatSwitchCount(1);    // Set a single D-Pad
    bleGamepadConfig.setIncludeZAxis(false);  // Simplify the HID report 
    bleGamepadConfig.setIncludeRxAxis(false); // By removing unused axis
    bleGamepadConfig.setIncludeRyAxis(false);
    bleGamepadConfig.setIncludeRzAxis(false);
    bleGamepadConfig.setIncludeSlider1(false);
    bleGamepadConfig.setIncludeSlider2(false);

    if(compressPowerpad)
    {
        bleGamepadConfig.setButtonCount(12);  
    }
    else
    {
        bleGamepadConfig.setButtonCount(6);  
    }

    bleGamepadConfig.setIncludeStart(true);
    bleGamepadConfig.setIncludeSelect(true);
    
    bleGamepadConfig.setAutoReport(false); // Manually handle reports, for performance

    bleGamepad.begin(&bleGamepadConfig);

    if (DEBUG)
    {
        Serial.println("##### Done Setup BLE");
    }
}

bool connected() // Check if Bluetooth connected
{
    return bleGamepad.isConnected();
}

void outputDirect(bool press, uint8_t input) // Output a button directly
{
    if(press)
        bleGamepad.press(input);
    else
        bleGamepad.release(input);
}

void inline pressSelect(bool input)
{
    if(input)
    {
        if(emulatorMapping)
            bleGamepad.press(BUTTON_5);
        else
            bleGamepad.pressSelect();
    }
    else
    {
        if(emulatorMapping)
            bleGamepad.release(BUTTON_5);
        else
            bleGamepad.releaseSelect();
    }
    
}

void outputGamepad(uint8_t gamepadData, uint8_t prevPadData) // Output using gamepad value data
{
    if((bitRead(gamepadData ,  7 - 0) == LOW) && (bitRead(prevPadData ,7 -  0) == HIGH))  // Inverted
    {
      bleGamepad.press(BUTTON_1);
      if (DEBUG)
      {
        Serial.println("# BTN: A Pressed");
      }
    }
    else if((bitRead(gamepadData , 7 - 0) == HIGH) && (bitRead(prevPadData , 7 - 0) == LOW)) 
    {
      bleGamepad.release(BUTTON_1);
      if (DEBUG)
      {
        Serial.println("# BTN: A Released");
      }
    }

    if((bitRead(gamepadData , 7 - 1) == LOW) && (bitRead(prevPadData , 7 - 1) == HIGH)) 
    {
      bleGamepad.press(BUTTON_4);
      if (DEBUG)
      {
        Serial.println("# BTN: B Pressed");
      }
    }  
    else if((bitRead(gamepadData , 7 - 1) == HIGH) && (bitRead(prevPadData , 7 - 1) == LOW)) 
    {
      bleGamepad.release(BUTTON_4);
      if (DEBUG)
      {
        Serial.println("# BTN: B Released");
      }
    }

    if((bitRead(gamepadData , 7 - 2) == LOW) && (bitRead(prevPadData , 7 - 2) == HIGH)) 
    {
      pressSelect(true);
      if (DEBUG)
      {
        Serial.println("# BTN: Select Pressed");
      }
    }
    else if((bitRead(gamepadData , 7 - 2) == HIGH) && (bitRead(prevPadData , 7 - 2) == LOW)) 
    {
      pressSelect(false);
      if (DEBUG)
      {
        Serial.println("# BTN: Select Released");
      }
    }

    if((bitRead(gamepadData , 7 - 3) == LOW) && (bitRead(prevPadData , 7 - 3) == HIGH))
    {
      bleGamepad.pressStart();
      if (DEBUG)
      {
        Serial.println("# BTN: Start Pressed");
      }
      
    }
    else if((bitRead(gamepadData , 7 - 3) == HIGH) && (bitRead(prevPadData , 7 - 3) == LOW))
    {
      bleGamepad.releaseStart();
      if (DEBUG)
      {
        Serial.println("# BTN: Start Released");
      }
    }

    // Check if DPad changed
    if( (bitRead(gamepadData , 7 - 4) != bitRead(prevPadData , 7 - 4)) 
      || (bitRead(gamepadData , 7 - 5) != bitRead(prevPadData , 7 - 5))
      || (bitRead(gamepadData , 7 - 6) != bitRead(prevPadData , 7 - 6)) 
      || (bitRead(gamepadData , 7 - 7) != bitRead(prevPadData , 7 - 7))) 
    {
      if (DEBUG)
      {
        Serial.print("# DPAD: ");
      }
        

      if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
      && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(1); // UP          (N)
        if (DEBUG)
        {
          Serial.println("1 U");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(2); // UP RIGHT    (NE)
        if (DEBUG)
        {
          Serial.println("2 UR");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(3); // RIGHT       (E)
        if (DEBUG)
        {
          Serial.println("3 R");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == LOW)) 
      {
        bleGamepad.setHat1(4); // DOWN RIGHT  (SE)
        if (DEBUG)
        {
          Serial.println("4 DR");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == HIGH) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(5); // DOWN        (S)
        if (DEBUG)
        {
          Serial.println("5 D");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == LOW)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(6);  // DOWN LEFT  (SW)
        if (DEBUG)
        {
          Serial.println("6 DL");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == HIGH) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(7); // LEFT        (W)
        if (DEBUG)
        {
          Serial.println("7 L");
        }
      }
      else if( (bitRead(gamepadData , 7 - 4) == LOW) && (bitRead(gamepadData , 7 - 5) == HIGH)
            && (bitRead(gamepadData , 7 - 6) == LOW) && (bitRead(gamepadData , 7 - 7) == HIGH)) 
      {
        bleGamepad.setHat1(8); // UP LEFT     (NW)
        if (DEBUG)
        {
          Serial.println("8 UL");
        }
      }
      else
      {
        bleGamepad.setHat1(0); // CENTRE      (C)
        if (DEBUG)
        {
          Serial.println("0 C");
        }
      }
    }

    bleGamepad.sendReport();

    if (DEBUG)
    {
      Serial.println(gamepadData, BIN);
    }
}



void resetAll()
{
    bleGamepad.setHat1(0);
    bleGamepad.releaseStart();
    pressSelect(false);
    bleGamepad.release(BUTTON_1);
    bleGamepad.release(BUTTON_4);
}

bool oddframe = false;

inline void CompressPowerpad(uint8_t btn)
{
    oddframe = !oddframe;
    if(oddframe)
    {
        if (btn == 0)
        {
            bleGamepad.setHat1(1);
        }
        else if (btn == 1)
        {
            bleGamepad.setHat1(3);
        }
        else if (btn == 2)
        {
            bleGamepad.setHat1(5);
        }
        else if (btn == 3)
        {
            bleGamepad.setHat1(7);
        }
        else if (btn == 4)
        {
            bleGamepad.press(BUTTON_1);
        }
        else if (btn == 5)
        {
            bleGamepad.press(BUTTON_4);
        }
    }
    else
    {
        if (btn == 6)
        {
            bleGamepad.setHat1(1);
        }
        else if (btn == 7)
        {
            bleGamepad.setHat1(3);
        }
        else if (btn == 8)
        {
            bleGamepad.setHat1(5);
        }
        else if (btn == 9)
        {
            bleGamepad.setHat1(7);
        }
        else if (btn == 10)
        {
            bleGamepad.press(BUTTON_1);
        }
        else if (btn == 11)
        {
            bleGamepad.press(BUTTON_4);
        }    
    }
}

void outputPowerpad(uint8_t powerpadData, uint8_t prevPadData) // Output using powerpad value data
{
    if(oddframe)
    {
        resetAll();
        pressSelect(true);

        if(powerpadData != prevPadData) // If state changed
        {
            for(int n = 0; n < 6; n++)
            {
                if( ( bitRead(powerpadData, 11 - n) == LOW) && ( bitRead(prevPadData, 11 - n) == HIGH)) // Inverted
                {
                    CompressPowerpad(PowerPadBtnMap[n]);
                    if (DEBUG)
                    {
                        Serial.print("# BTN: ");
                        Serial.print(PowerPadBtnMap[n]);
                        Serial.println(" Pressed");
                    }
                }
            }
        }
    }
    else
    {
        for(int n = 6; n < 12; n++)
        {
            if( ( bitRead(powerpadData, 11 - n) == LOW) && ( bitRead(prevPadData, 11 - n) == HIGH)) // Inverted
            {
                CompressPowerpad(PowerPadBtnMap[n]);
                if (DEBUG)
                {
                    Serial.print("# BTN: ");
                    Serial.print(PowerPadBtnMap[n]);
                    Serial.println(" Pressed");
                }
            }
        }
    }
}

void updatePad() // Send bluetooth update report
{
    bleGamepad.sendReport();
}
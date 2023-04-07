-----------
Description 
-----------
Custom kit initially designed for Wangaratta District Specialist School. A device to use a NES Zapper with any Bluetooth receiver, including BlueRetro, To be used with with custom patched ROMs from @neslcdmod.

-----------
Parts 
-----------
1x NES controller extension cable  
1x ESP32 DevKitC  
  
Also required: 1x NES Zapper (Tested with a Tomee Zapp Gun that works better with non-CRT displays)  


------------------
Instructions 
------------------
1. Cut the NES controller extension cable in half and keep the socket end.

2. Strip 4cm of outer insulation from the NES controller extension cable.

3. Dissassemble the socket of the NES controller extension cable and carefully remove the pins for the CLOCK and LATCH pins, or any pins marked NC on the diagram below, and move them to the position of the Light and Trigger pins of the diagram below.
NOTE: This is usually required because most NES controller extension cables leave the pins for Light and Trigger disconnected, if your cable has all pins wired, skip this step.

4. Strip the wires for the pins GND, 5V, Light and Trigger shown on the diagram below, use a multimeter to test which wire colour responds to which pin for your cable.

5. Crimp the wires with female dupont terminals and connect them to the IO pins shown below, or solder them directly to the pins of the ESP32 DevKit

TODO: Additional setup and use instructions

-------------
Pin map
-------------


ESP32   |   Zapper   |  (Colour)     
--------|------------|--------------
21      |   Light    |   (Yellow)    
19      |   Trigger  |   (Black)     
5V      |   5V       |  (Red)          
GND     |   GND      |  (White)        

```
      .--               
GND --|O \                          (White)
NC <- |O O\ -- +5V                  (Red)
NC <- |O O| <- Pin 21 / Light       (Yellow)   
NC -> |O O| <- Pin 19 / Trigger     (Black)      
       ---
```

------------------------
CREDITS / LIBARIES 
------------------------
Only possible due to ROM patches from NESLCDMOD  
Utilises: https://github.com/lemmingDev/ESP32-BLE-Gamepad
Which relies on -> https://github.com/h2zero/NimBLE-Arduino
      '---'       

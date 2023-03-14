-----------
--PIN MAP--
-----------

ESP32   /   Zapper     (Colour)    /   Level Shifter
--------------------------------------------------
21      /   Light   (Yellow)    /   3V3->5V
19      /   Trigger   (Black)     /   3V3->5V
5V      /   5V      (Red)       /   
GND     /   GND     (White)     /   

      .--               
GND --|O \            White
NC <- |O O\ -- +5V    Red
NC <- |O O| <- D3     
NC -> |O O| <- D4        
      '---'       
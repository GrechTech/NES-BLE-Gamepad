#include "main.h"

const uint16_t period = 1000; // ~ two seconds
unsigned int outputCount = 0;
uint16_t prevData = 0;

// Press every powerpad button, incrementing every 2 seconds
uint16_t TestSequence()
{
  uint16_t outputData = 0;
  
  if(outputCount > period * 12)
    outputCount = 0;
    
  for(int n = 0; n < 12; n++)
  {
    if(outputCount >  (period * n) && outputCount <= period * (n + 1) )
    {
      outputData = pow(2, n);
      outputData = ~outputData;
      if(DEBUG && outputData != prevData)
      {
        Serial.print("Unit: ");Serial.print(n);
        Serial.print(" Data: ");Serial.println(outputData, BIN);
      }
    }
  }

  outputCount++;
  prevData = outputData;

  return outputData;
}
#include "main.h"

unsigned int outputCount = 0;
uint16_t prevData = 0;

uint16_t TestSequence()
{
  uint16_t outputData = 0;
  const uint16_t period = 1000; // ~ two seconds
  //const uint16_t period = 20; // ~ test
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

  if(outputCount > period * 12)
    outputCount = 0;

  if(outputData != prevData) // If state changed
      prevData = outputData;

  return outputData;
}
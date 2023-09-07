#include "main.h"

const uint16_t period = 2000; // ~ two seconds

// Press every powerpad button, incrementing every 2 seconds
uint16_t testPowerpad()
{
  uint16_t outputData = 0;
  static uint16_t prevData = 0;
  static unsigned int outputCount = 0;
  
  if(outputCount > period * 12)
    outputCount = 0;
    
  for(int n = 0; n < 12; n++)
  {
    if(outputCount >  (period * n) && outputCount <= period * (n + 1) )
    {
      outputData = pow(2, n);
      outputData = ~outputData;
    }
  }

  outputCount++;
  prevData = outputData;

  return outputData;
}
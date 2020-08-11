// DimmingCurves.h

#ifndef _DIMMING_CURVES_H_
#define _DIMMING_CURVES_H_

#include "mbed.h"

class DimmingCurves
{
public:
  DimmingCurves(const uint8_t index = 0);

  uint8_t selectCurve(const uint8_t index);

  uint8_t getCurrentCurve() { return currentCurve; }

  float getDimming(const uint8_t hour);

protected:
  uint8_t currentCurve;
  uint8_t maxIndex;

  static const uint8_t curves[][12];
};

#endif
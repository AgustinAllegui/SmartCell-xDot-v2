// PhotoCell.h

#ifndef _PHOTO_CELL_H_
#define _PHOTO_CELL_H_

#include "mbed.h"

class PhotoCell
{
public:
  PhotoCell(PinName pin_, const float onThreshold_, const float offThreshold_);

  bool shouldBeOn();
  float read();

  void setThreshold(const float onThreshold_, const float offThreshold_);

protected:
  AnalogIn pin;
  float onThreshold;
  float offThreshold;

  bool lastState;
};

#endif

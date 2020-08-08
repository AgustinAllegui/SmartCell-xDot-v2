// LightController.h

#ifndef _LIGHT_CONTROLLER_H_
#define _LIGHT_CONTROLLER_H_

#include "mbed.h"
#include "PhotoCell.h"
#include "DimmingCurves.h"

class LightController
{
public:
  enum OpMode
  {
    Manual,
    AutoPhotoCell,
    AutoCurve
  };

  LightController(PhotoCell *photoCell_, DimmingCurves *dimmingCurves_, const LightController::OpMode opMode_);

  float getDimming(const uint8_t hour);
  void setManualDimming(float dimLevel);
  void setOpMode(const OpMode opMode_);
  LightController::OpMode getMode();

private:
  PhotoCell *photoCell;
  DimmingCurves *dimmingCurves;
  OpMode currentMode;

  float manualDimLevel;
};

#endif
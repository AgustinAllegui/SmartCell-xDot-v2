// LightController.h

#ifndef _LIGHT_CONTROLLER_H_
#define _LIGHT_CONTROLLER_H_

#include "mbed.h"
#include "PhotoCell.h"
#include "DimmingCurves.h"

class LightController
{
public:
  enum class OpMode : uint8_t
  {
    Manual = 0x00,
    AutoPhotoCell = 0x01,
    AutoCurve = 0x02
  };

  LightController(PhotoCell *photoCell_, DimmingCurves *dimmingCurves_, const LightController::OpMode opMode_);

  float getDimming(const uint8_t hour);
  void setManualDimming(float dimLevel);
  void setOpMode(const OpMode opMode_);
  LightController::OpMode getMode();
  void printMode();

private:
  PhotoCell *photoCell;
  DimmingCurves *dimmingCurves;
  OpMode currentMode;

  float manualDimLevel;
};

#endif

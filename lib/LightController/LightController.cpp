// LightController.cpp

#include "LightController.h"

LightController::LightController(PhotoCell *photoCell_, DimmingCurves *dimmingCurves_, const LightController::OpMode opMode_)
    : photoCell(photoCell_),
      dimmingCurves(dimmingCurves_),
      currentMode(opMode_),
      manualDimLevel(1.0f)
{
}

void LightController::setOpMode(const OpMode opMode_)
{
  currentMode = opMode_;
}

LightController::OpMode LightController::getMode()
{
  return currentMode;
}

void LightController::printMode()
{
  switch (currentMode)
  {
  default:
  case LightController::Manual:
    logInfo("Mode ============ Manual Dimming");
    break;
  case LightController::AutoCurve:
    logInfo("Mode ============ Curve %u", dimmingCurves->getCurrentCurve());
    break;
  case LightController::AutoPhotoCell:
    logInfo("Mode ============ PhotoCell");
    logInfo("lux ============= %.0f%", photoCell->getLastRead() * 100);
    break;
  }
}

float LightController::getManualDimLevel()
{
  return manualDimLevel;
}

void LightController::setManualDimming(float dimLevel)
{
  if (dimLevel > 1.0f)
    dimLevel = 1;
  if (dimLevel < 0.0f)
    dimLevel = 0;

  manualDimLevel = dimLevel;
}

float LightController::getDimming(const uint8_t hour)
{
  switch (currentMode)
  {
  default:
  case LightController::Manual:
    return manualDimLevel;
    break;

  case LightController::AutoPhotoCell:
    return (photoCell->shouldBeOn() ? 1.0f : 0.0f);
    break;

  case LightController::AutoCurve:
    return dimmingCurves->getDimming(hour);
    break;
  }
}

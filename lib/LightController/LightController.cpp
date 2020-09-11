// LightController.cpp

#include "LightController.h"

LightController::LightController(PhotoCell *photoCell_, DimmingCurves *dimmingCurves_, DimmingDemo *dimmingDemo_, const LightController::OpMode opMode_)
    : photoCell(photoCell_),
      dimmingCurves(dimmingCurves_),
      dimmingDemo(dimmingDemo_),
      currentMode(opMode_),
      manualDimLevel(1.0f)
{
}

void LightController::setOpMode(const OpMode opMode_)
{
  currentMode = opMode_;
}

uint8_t LightController::getMode()
{
  return static_cast<uint8_t>(currentMode);
}

void LightController::printMode()
{
  switch (currentMode)
  {
  default:
  case OpMode::Manual:
    logInfo("Mode ============ Manual Dimming");
    break;
  case OpMode::AutoCurve:
    logInfo("Mode ============ Curve %u", dimmingCurves->getCurrentCurve());
    break;
  case OpMode::AutoPhotoCell:
    logInfo("Mode ============ PhotoCell");
    logInfo("lux ============= %.0f%", photoCell->getLastRead() * 100);
    break;
  case OpMode::Demo:
    logInfo("Mode ============ Demo");
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
  case OpMode::Manual:
    return manualDimLevel;
    break;

  case OpMode::AutoPhotoCell:
    return (photoCell->shouldBeOn() ? 1.0f : 0.0f);
    break;

  case OpMode::AutoCurve:
    return dimmingCurves->getDimming(hour);
    break;

  case OpMode::Demo:
    return dimmingDemo->getDimming();
    break;
  }
}

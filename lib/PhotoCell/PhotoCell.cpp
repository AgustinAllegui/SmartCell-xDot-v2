// PhotoCell.cpp

#include "PhotoCell.h"

PhotoCell::PhotoCell(PinName pin_, const float onThreshold_, const float offThreshold_)
    : pin(pin_), lastState(false), lastRead(0)
{
  setThreshold(onThreshold_, offThreshold_);
}

void PhotoCell::setThreshold(const float onThreshold_, const float offThreshold_)
{
  onThreshold = onThreshold_;
  if (onThreshold_ <= offThreshold_)
  {
    offThreshold = offThreshold_;
  }
  else
  {
    offThreshold = onThreshold_;
  }
}

/*
 *  Lee el valor de la fotocelda
 *  Realiza un primedio de 10 puntos
 */
float PhotoCell::read(const uint8_t ammount)
{
  float accumulator = 0;
  for (uint8_t i = 1; i < ammount; i++)
  {
    accumulator += (pin.read() / ammount);
  }

  lastRead = accumulator;
  return accumulator;
}

float PhotoCell::getLastRead()
{
  return lastRead;
}

/*
 * Devuelve true si la fotocelda indica 
 * que deberia encenderse la luz.
 */
bool PhotoCell::shouldBeOn()
{
  float currentRead = read();
  if (lastState)
  {
    if (currentRead < offThreshold)
    {
      lastState = false;
      return false;
    }
    else
    {
      lastState = true;
      return true;
    }
  }
  else
  {
    if (currentRead > onThreshold)
    {
      lastState = true;
      return true;
    }
    else
    {
      lastState = false;
      return false;
    }
  }
}

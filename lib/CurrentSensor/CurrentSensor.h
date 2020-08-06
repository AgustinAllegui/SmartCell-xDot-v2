// CurrentSensor.h
// sensor de corriente de nodo SmartCell-xdot (desconozco el modelo del chip)

#ifndef _CURRENT_SENSOR_H_
#define _CURRENT_SENSOR_H_

#include "mbed.h"

const float curr_sens = 1.352; // I/Vpp [mA/mV = A/V = 1/ohm]
const float thr_curr = .15;		 // zona muerta del sensor [Ampere]

class CurrentSensor
{
public:
	CurrentSensor(PinName pin);
	float getCurrent();

private:
	Timer timer;
	AnalogIn currentIn;
};

#endif
/**
 * @file smartCell_util.h
 * @brief Funciones utiles para SmartCell
 * @version 0.1
 * @date 2020-08-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */

// SmartCell_util.h

#ifndef SMART_CELL_UTIL_H
#define SMART_CELL_UTIL_H

#include "example_config.h"
#include "mbed.h"
#include "mDot.h"
#include "dot_util.h"

//#include "EventSlot.h"

// declaro el objeto dot para tener acceso desde las funciones
extern mDot *dot;

// Funciones para Manejo del RTC

bool timeIsSynced();
bool syncTime(const uint8_t attempts = 1, const float timezone = TIME_ZONE);
uint8_t getHour();
void setManualTime(const uint32_t timestamp);
void printTime();

// funciones para envio de datos

void send_lightStatus(const float dimming, const float power, const float energy);
void send_smartCellConfig(const uint8_t lightMode, const uint8_t manualDimming, const uint16_t loopDelay);
void send_currentTime();

#endif
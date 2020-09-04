//-------------------------------------------------------------
// Activar lectura de Configuracon de memoria no volatil

#ifndef _VERSION_CONFIG_H_
#define _VERSION_CONFIG_H_

//-------------------------------------------------------------
// Version

#define PROMATIX_VERSION_MAJOR 2
#define PROMATIX_VERSION_MINOR 2
#define PROMATIX_VERSION_PATCH 0

//-------------------------------------------------------------
// Direcciones en memoria no volatil

#define DIR_PROMATIX_VERSION_MAJOR 0x00
#define DIR_PROMATIX_VERSION_MINOR 0x01
#define DIR_PROMATIX_VERSION_PATCH 0x02

#define DIR_LOOP_DELAY 0x10
#define DIR_OP_MODE 0x12
#define DIR_CURVE 0x13
#define DIR_MANUAL_DIMMING 0x14

//-------------------------------------------------------------
// Actives generation of test data (para version PULSE_METER)

// 0: deactivated
// 1: activated
#if !defined(TEST_DATA_GEN)
#define TEST_DATA_GEN 0
#endif

#endif

//-------------------------------------------------------------
// secuencias para los leds

#define LED_SEQUENCE_OK \
    {                   \
        0.2, 1.7        \
    }
#define LED_SEQUENCE_ERROR_1 \
    {                        \
        0.2, 0.2, 0.2, 1.4   \
    }
#define LED_SEQUENCE_ERROR_2         \
    {                                \
        0.2, 0.2, 0.2, 0.2, 0.2, 1.0 \
    }
#define LED_SEQUENCE_ERROR_3                   \
    {                                          \
        0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.6 \
    }

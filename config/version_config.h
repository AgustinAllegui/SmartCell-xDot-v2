//-------------------------------------------------------------
// Activar lectura de Configuracon de memoria no volatil

#ifndef _VERSION_CONFIG_H_
#define _VERSION_CONFIG_H_

//-------------------------------------------------------------
// Version

#define PROMATIX_VERSION_MAJOR 2
#define PROMATIX_VERSION_MINOR 6
#define PROMATIX_VERSION_PATCH 0

//-------------------------------------------------------------
// Direcciones en memoria no volatil

#define DIR_PROMATIX_VERSION_MAJOR 0x00 // 1 Byte
#define DIR_PROMATIX_VERSION_MINOR 0x01 // 1 Byte
#define DIR_PROMATIX_VERSION_PATCH 0x02 // 1 Byte

#define DIR_LOOP_DELAY 0x10     // 2 Bytes
#define DIR_OP_MODE 0x12        // 1 Byte
#define DIR_CURVE 0x13          // 1 Byte
#define DIR_MANUAL_DIMMING 0x14 // 1 Byte
#define DIR_CUSTOM_CURVE 0x15   // 12 Bytes
#define DIR_ON_OFF_TIME 0x21    // 4 bytes


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
        0.2, 2.7        \
    }
#define LED_SEQUENCE_ERROR_1 \
    {                        \
        0.2, 0.5, 0.2, 3.0   \
    }
#define LED_SEQUENCE_ERROR_2         \
    {                                \
        0.2, 0.5, 0.2, 0.5, 0.2, 3.0 \
    }
#define LED_SEQUENCE_ERROR_3                   \
    {                                          \
        0.2, 0.5, 0.2, 0.5, 0.2, 0.5, 0.2, 3.0 \
    }

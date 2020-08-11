//-------------------------------------------------------------
// Activar lectura de Configuracon de memoria no volatil

#ifndef _VERSION_CONFIG_H_
#define _VERSION_CONFIG_H_

//-------------------------------------------------------------
// Version

#define PROMATIX_VERSION_MAYOR 2
#define PROMATIX_VERSION_MINOR 0
#define PROMATIX_VERSION_PATCH 0

//-------------------------------------------------------------

// 0: deactivated
// 1: activated
#if !defined(ENABLE_READ_NON_VOLATILE)
#define ENABLE_READ_NON_VOLATILE 1
#endif

//-------------------------------------------------------------
// Direcciones en memoria no volatil

#define DIR_PROMATIX_VERSION_MAYOR 0x00
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

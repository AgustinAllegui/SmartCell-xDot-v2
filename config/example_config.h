#ifndef __EXAMPLE__CONFIG_H__
#define __EXAMPLE__CONFIG_H__

//-------------------------------------------------------------
// Seleccion de version

#define OTA_EXAMPLE 1          // see ota_example.cpp
#define AUTO_OTA_EXAMPLE 2     // see auto_ota_example.cpp
#define MANUAL_EXAMPLE 3       // see manual_example.cpp
#define PEER_TO_PEER_EXAMPLE 4 // see peer_to_peer_example.cpp
#define CLASS_C_EXAMPLE 5      // see class_c_example.cpp
#define CLASS_B_EXAMPLE 6      // see class_b_example.cpp
#define FOTA_EXAMPLE 7         // see fota_example.cpp

#define PULSE_METER 8 // see pulse_meter.cpp
#define LUMINARY 9    // see luminary.cpp

// the active example is the one that will be compiled
#if !defined(ACTIVE_EXAMPLE)
#define ACTIVE_EXAMPLE LUMINARY
#endif

// the active channel plan is the one that will be compiled
// options are :
//      CP_US915
//      CP_AU915
//      CP_EU868
//      CP_KR920
//      CP_AS923
//      CP_AS923_JAPAN
//      CP_IN865
#if !defined(CHANNEL_PLAN)
#define CHANNEL_PLAN CP_AU915
#endif

//-------------------------------------------------------------
// Time zone
#define TIME_ZONE -3

//-------------------------------------------------------------
// Habilitar Join

// 0: deactivated
// 1: activated
#define ENABLE_JOIN 1

//-------------------------------------------------------------
// define App Eui, App key y Sub band

#define APP_EUI {0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x02}

// nodo xDot
#define APP_KEY {0x1d, 0xbf, 0x53, 0x7d, 0xc8, 0xb3, 0xa0, 0x5b, 0x51, 0xd4, 0x39, 0x34, 0xce, 0x6b, 0x8e, 0xe5}

/*	sub band:
 *	0: Auto
 *	1: Yeap
 *	2: ChirpStack
 */

#define LORA_SUB_BAND 2

//-------------------------------------------------------------
// Activar lectura de Configuracon de EEPROM

// 0: deactivated
// 1: activated
#if !defined(READ_NON_VOLATILE)
#define READ_NON_VOLATILE 0
#endif

//-------------------------------------------------------------
// Actives generation of test data (para version PULSE_METER)

// 0: deactivated
// 1: activated
#if !defined(TEST_DATA_GEN)
#define TEST_DATA_GEN 0
#endif

//-------------------------------------------------------------
#endif

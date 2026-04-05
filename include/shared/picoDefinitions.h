/**
 * @file picoDefinitions.h
 * @brief Defines values related to implementation of the Raspberry Pi Pico 2.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/3/26
 */
#ifndef PICO_DEFS
#define PICO_DEFS

#include "hardware/pio.h"

// Raspberry Pi Pico 2 memory constants
#define PICO2_FLASH_BYTES 4194304
#define PICO2_SRAM_BYTES  532480

// Debug Status
#define DEBUG // Enables debug functionality

/* Hardware Values*/


/* Port Definitions */
// Display Definitions
#define DISPLAY_DC 0
#define DISPLAY_CS 1
#define DISPLAY_SCK 2
#define DISPLAY_DIN 3
#define DISPLAY_RESET 4
#define DISPLAY_BLACKLIGHT 5

// RF Impedance I2C
#define RF_SDA 6
#define RF_SCL 7

// Vacuum Gauge
#define GAUGE_DI_PIN 8
#define GAUGE_RO_PIN 9
#define GAUGE_TR_RE 10

// Pressure Pump
#define PUMP_TR_RE 11
#define PUMP_DI_PIN 12
#define PUMP_RO_PIN 13

// Unused
#define UNUSED1 14
#define UNUSED2 15

// AlicatMFC 1
#define ALICAT_1_TX 16
#define ALICAT_1_RX 17

// AlicatMFC 2
#define ALICAT_2_TX 18
#define ALICAT_2_RX 19

// USB 2
#define USB2_D_PLUS 20
#define USB2_D_MINUS 21

// USB 3
#define USB3_D_PLUS 22
#define USB3_D_MINUS 23

// Switches
#define SWITCH1 22
#define SWITCH2 28

/* PIO Definitions */
#define ALICAT1_PIO pio0
#define ALICAT1_SM1 0
#define ALICAT1_SM2 1

#define ALICAT2_PIO pio1
#define ALICAT2_SM1 0
#define ALICAT2_SM2 1

#endif
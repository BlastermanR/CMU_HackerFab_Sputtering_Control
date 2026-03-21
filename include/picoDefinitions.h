#ifndef PICO_DEFS
#define PICO_DEFS

/**
 * Defines values related to implementation of the Raspberry Pi
 * Pico 2.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/3/26
 */

// Debug Status
#define DEBUG // Enables debug functionality

// Port Definitions
#define UART0_TX 16
#define UART0_RX 17
#define PIO_UART_TX 18
#define PIO_UART_RX 19

// DEVICE PORTS
// Vacuum Pump
#define P1_DI_PIN 8
#define P1_RO_PIN 9
#define P1_TR_RE 10

// Pressure Gauge
#define P2_DI_PIN 12
#define P2_RO_PIN 13
#define P2_TR_RE 14

// AlicatMFC 1
#define ALICAT_1_TX 16
#define ALICAT_1_RX 17

// AlicatMFC 2
#define ALICAT_2_TX 18
#define ALICAT_2_RX 19

#endif
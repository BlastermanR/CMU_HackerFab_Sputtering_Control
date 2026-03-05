#ifndef PICO_DEFS
#define PICO_DEFS

/**
 * Defines values related to implementation of the Raspberry Pi
 * Pico 2.
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/3/26
 */

// Port Definitions
#define UART0_TX 16
#define UART0_RX 17
#define PIO_UART_TX 18
#define PIO_UART_RX 19

// DEVICE PORTS
// Pressure Gauge
#define P1_DI_PIN 8
#define P1_RO_PIN 9
#define P1_RE_PIN 10
#define P1_DE_PIN 11

// Vacuum Pump
#define P2_DI_PIN 12
#define P2_RO_PIN 13
#define P2_RE_PIN 14
#define P2_DE_PIN 15

// AlicatMFC 1
#define ALICAT_1_TX 16
#define ALICAT_1_RX 17

// AlicatMFC 2
#define ALICAT_2_TX 18
#define ALICAT_2_RX 19

#endif
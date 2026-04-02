// GlobalDevices.h
#ifndef GLOBALDEVICES_H
#define GLOBALDEVICES_H

#include "AlicatMFC.h"
#include "PfeifferGauge.h"
#include "PfeifferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "PIO_UART.h"
#include "HardwareUART.h"
#include "picoDefinitions.h"

// ALICAT 1 Devices
extern PioUart     alicat1Uart;
extern RS232Device alicat1Device;
extern AlicatMFC   mfc1;

// ALICAT 2 Devices
extern PioUart     alicat2Uart;
extern RS232Device alicat2Device;
extern AlicatMFC   mfc2;

// Gauge Devices (uart0)
extern HardUart      gaugeUart;
extern RS485Device   gaugeDevice;
extern PfeifferGauge gauge;

// Pump Devices (uart1)
extern HardUart     pumpUart;
extern RS485Device  pumpDevice;
extern PfeifferPump pump;

#endif // GLOBALDEVICES_H
// GlobalDevices.cpp
#include "GlobalDevices.h"
#include "picoDefinitions.h"

// Define the ALICAT UART and Devices
PioUart     alicat1Uart(ALICAT1_PIO, ALICAT1_SM1, ALICAT1_SM2, ALICAT_1_TX, ALICAT_1_RX, 9600);
RS232Device alicat1Device(&alicat1Uart);
AlicatMFC   mfc1(&alicat1Device);

PioUart     alicat2Uart(ALICAT2_PIO, ALICAT2_SM1, ALICAT2_SM2, ALICAT_2_TX, ALICAT_2_RX, 9600);
RS232Device alicat2Device(&alicat2Uart);
AlicatMFC   mfc2(&alicat2Device);

// Initialize Gauge UART and Device on uart1 (GPIO 8/9 are UART1 TX/RX)
HardUart      gaugeUart(uart1, GAUGE_DI_PIN, GAUGE_RO_PIN, 9600);
RS485Device   gaugeDevice(&gaugeUart, GAUGE_TR_RE);
PfeifferGauge gauge(&gaugeDevice);

// Initialize Pump UART and Device on uart0 (GPIO 12/13 are UART0 TX/RX)
HardUart     pumpUart(uart0, PUMP_DI_PIN, PUMP_RO_PIN, 9600);
RS485Device  pumpDevice(&pumpUart, PUMP_TR_RE);
PfeifferPump pump(&pumpDevice);
/**
 * main.cpp
 * @brief Main entry point for the Sputtering system on Core 0.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <stdio.h>
#include "AlicatMFC.h"
#include "Core1Main.h"
#include "Intercore.h"
#include "PIO_UART.h"
#include "PfiefferGauge.h"
#include "PfiefferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "USBSerial.h"
#include "HardwareUART.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

int main()
{
    // Enable IO
    stdio_init_all();

    // Launch Core 1
    multicore_launch_core1(core1_entry);

    /**
     * Define Devices
     */

    // Setup the serial communication
    USBSerial pcTerminal;
    pcTerminal.begin();

    // Define the ALICAT UART and Devices
    PioUart     alicat1Uart(ALICAT1_PIO, ALICAT1_SM1, ALICAT1_SM2, ALICAT_1_TX, ALICAT_1_RX, 9600);
    RS232Device alicat1Device(&alicat1Uart);
    AlicatMFC   mfc1(&alicat1Device);

    PioUart     alicat2Uart(ALICAT2_PIO, ALICAT2_SM1, ALICAT2_SM2, ALICAT_2_TX, ALICAT_2_RX, 9600);
    RS232Device alicat2Device(&alicat2Uart);
    AlicatMFC   mfc2(&alicat2Device);

    // Initialize Gauge UART and Device on uart0
    HardUart      gaugeUart(uart0, GAUGE_DI_PIN, GAUGE_RO_PIN, 9600);
    RS485Device   gaugeDevice(&gaugeUart, GAUGE_TR_RE);
    PfiefferGauge gauge(&gaugeDevice);

    // Initialize Pump UART and Device on uart1
    HardUart     pumpUart(uart1, PUMP_DI_PIN, PUMP_RO_PIN, 9600);
    RS485Device  pumpDevice(&pumpUart, PUMP_TR_RE);
    PfiefferPump pump(&pumpDevice);

    // Set up parsing callback
    pcTerminal.setCallback(
        [&pcTerminal](const std::string &command)
        {
            // Echo the command back to the PC
            pcTerminal.println(command.c_str());

            // Future: Pass the command to the Sputtering Manager on Core 1
        });

    /**
     * Initialize Devices
     */
    mfc1.init();
    mfc2.init();
    gauge.init();
    pump.init();

    /**
     * Loop through until program exit and update devices
     */
    bool run{true};

    while (run)
    {
        pcTerminal.update();
        mfc1.update();
        mfc2.update();
        gauge.update();
        pump.update();
    }
}

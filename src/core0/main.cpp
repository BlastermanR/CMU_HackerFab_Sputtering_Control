/**
 * main.cpp
 * @brief Main entry point for the Sputtering system on Core 0.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "AlicatMFC.h"
#include "HardwareUART.h"
#include "PIO_UART.h"
#include "PfiefferGauge.h"
#include "PfiefferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "USBSerial.h"
#include "Core1Main.h"
#include "Intercore.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <stdio.h>

// Define the global queue instance
queue_t core_queue;

int main()
{
    stdio_init_all();

    // Initialize the inter-core queue before launching Core 1
    queue_init(&core_queue, sizeof(DataPacket), 10);

    // Launch Core 1
    multicore_launch_core1(core1_entry);

    // Setup the serial communication
    USBSerial pcTerminal;
    pcTerminal.begin();

    // Dynamically define the UART and Devices
    PioUart     alicatUart(pio0, 0, 1, ALICAT_1_TX, ALICAT_1_RX, 9600);
    RS232Device alicatDevice(&alicatUart);
    AlicatMFC   mfc(&alicatDevice);

    // Initialize Gauge UART and Device on uart0
    HardUart      gaugeUart(uart0, GAUGE_DI_PIN, GAUGE_RO_PIN, 9600);
    RS485Device   gaugeDevice(&gaugeUart, GAUGE_TR_RE);
    PfiefferGauge gauge(&gaugeDevice);

    // Initialize Pump UART and Device on uart1
    HardUart     pumpUart(uart1, PUMP_DI_PIN, PUMP_RO_PIN, 9600);
    RS485Device  pumpDevice(&pumpUart, PUMP_TR_RE);
    PfiefferPump pump(&pumpDevice);

    // Set up parsing callback
    pcTerminal.setCallback([&pcTerminal](const std::string &command) {
        // Echo the command back to the PC
        pcTerminal.println(command.c_str());

        // Future: Pass the command to the Sputtering Manager on Core 1
    });

    mfc.init();
    gauge.init();
    pump.init();

    // Store the time we last sent a message
    uint32_t lastSendTime = to_ms_since_boot(get_absolute_time());

    while (true)
    {
        pcTerminal.update();
        mfc.update();
        gauge.update();
        pump.update();

        // Get the current time
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());

        // Check if 5 seconds have passed since the last send
        if (currentTime - lastSendTime >= 3000)
        {
#ifdef DEBUG
            gauge.sendMessage("0020074002=?107\r");
            
            // Push a test packet to the queue
            DataPacket testPacket = {1, 42.0f};
            queue_try_add(&core_queue, &testPacket);
#endif
            // Reset the timer
            lastSendTime = currentTime;
        }
    }
}

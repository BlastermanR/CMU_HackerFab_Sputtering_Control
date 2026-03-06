#include <stdio.h>
#include "pico/stdlib.h"

#include "picoDefinitions.h"
#include "HardwareUART.h"
#include "PIO_UART.h"
#include "AlicatMFC.h"
#include "USBSerial.h"

// Define USB Serial
USBSerial pcTerminal;

void blinkTest()
{
    // PICO_DEFAULT_LED_PIN is automatically set to 25 for the Pico 2
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) 
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250); // Faster blink for the faster chip!
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}

void onPcCommand(const std::string& command) 
{
    pcTerminal.println(command.c_str());
}

int main()
{
    stdio_init_all();

    //blinkTest();

    // Setup the serial communication
    pcTerminal.begin();
    pcTerminal.setCallback(onPcCommand);
    pcTerminal.println("Enter a setpoint to send to the mass flow controller:");

    // The Hardware UART (Using uart0, TX on GPIO 0, RX on GPIO 1, 9600 baud)
    HardUart hardwarePort(uart0, ALICAT_1_TX, ALICAT_1_RX, 9600);

    // Define the Alicat Device
    AlicatMFC massFlowController(&hardwarePort);

    // Alicat Test message
    const char* s = "A\r"; 
    
    // Store the time we last sent a message
    uint32_t last_send_time = to_ms_since_boot(get_absolute_time());

    while (true) 
    {
        pcTerminal.update();
        
        // Get the current time
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        // Check if 5 seconds have passed since the last send
        if (current_time - last_send_time >= 5000) 
        {
            massFlowController.sendMessage(s);
            
            // Reset the timer
            last_send_time = current_time;
        }
    }
}

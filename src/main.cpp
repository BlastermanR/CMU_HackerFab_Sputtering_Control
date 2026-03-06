#include <stdio.h>
#include "pico/stdlib.h"

#include "picoDefinitions.h"
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

void onPcCommand(const std::string& command) {
    pcTerminal.println(command.c_str());

    // Convert string to int and send to the physical device
    int setpoint = std::stoi(command);
    
    // TODO
    // Pass to Alicat
}

int main()
{
    //blinkTest();

    pcTerminal.begin();
    pcTerminal.setCallback(onPcCommand);
    
    pcTerminal.println("Enter a setpoint to send to the mass flow controller:");

    while (true) 
    {
        pcTerminal.update();
    }
}

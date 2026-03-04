#include <stdio.h>
#include "pico/stdlib.h"

#include "picoDefinitions.h"

void setupUart()
{
    uart_init(uart0, 115200);

    gpio_set_function(UART0_TX, GPIO_FUNC_UART); // TX
    gpio_set_function(UART0_RX, GPIO_FUNC_UART); // RX
}

void setupPIOUart()
{

}

void blinkTest()
{
    // PICO_DEFAULT_LED_PIN is automatically set to 25 for the Pico 2
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250); // Faster blink for the faster chip!
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}

int main()
{
    //setupUart();

    //setupPIOUart();

    blinkTest();
    
    while (true) 
    {
        // Code Here
    }
}

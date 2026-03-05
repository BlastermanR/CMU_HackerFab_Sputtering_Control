#include <stdio.h>
#include "pico/stdlib.h"

#include "picoDefinitions.h"

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

int main()
{
    //blinkTest();
    
    while (true) 
    {
        // Code Here
    }
}

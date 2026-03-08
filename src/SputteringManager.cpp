#include "SputteringManager.h"
#include "picoDefinitions.h"
#include <stdio.h>

SputteringManager::SputteringManager() 
    : alicatUart(uart0, ALICAT_1_TX, ALICAT_1_RX, 9600), 
      mfc(&alicatUart)
{
    // Constructor
}

SputteringManager::~SputteringManager() 
{
    // Intentionally left empty
}

void SputteringManager::init() 
{
    printf("Sputtering Manager: Initializing...\n");
    mfc.init();
}

void SputteringManager::update() 
{
    mfc.update();
    // Add logic here to manage other devices and control system states
}

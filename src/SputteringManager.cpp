#include "SputteringManager.h"
#include "picoDefinitions.h"
#include <stdio.h>

SputteringManager::SputteringManager() 
    : alicatUart(nullptr), 
      mfc(nullptr)
{
    // Constructor
}

SputteringManager::~SputteringManager() 
{
    if (mfc) 
    {
        delete mfc;
    }
    if (alicatUart) 
    {
        delete alicatUart;
    }
}

void SputteringManager::init() 
{
    printf("Sputtering Manager: Initializing...\n");
    
    // Dynamically define the UART first
    alicatUart = new HardUart(uart0, ALICAT_1_TX, ALICAT_1_RX, 9600);
    
    // Then define the device implementation mapped to that UART
    mfc = new AlicatMFC(alicatUart);
    mfc->init();
}

void SputteringManager::update() 
{
    if (mfc) 
    {
        mfc->update();
    }
    // Add logic here to manage other devices and control system states
}

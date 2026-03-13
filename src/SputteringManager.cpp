#include "SputteringManager.h"
#include "picoDefinitions.h"
#include "PIO_UART.h"
#include <stdio.h>
#include "pico/stdlib.h"

SputteringManager::SputteringManager() 
    : alicatUart(nullptr), 
      mfc(nullptr),
      gaugeUart(nullptr),
      gauge(nullptr)
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
    if (gauge)
    {
        delete gauge;
    }
    if (gaugeUart)
    {
        delete gaugeUart;
    }
}

void SputteringManager::init() 
{
    printf("Sputtering Manager: Initializing...\n");
    
    // Dynamically define the UART first
    alicatUart = new PioUart(pio0, 0, 1, ALICAT_1_TX, ALICAT_1_RX, 9600);
    
    // Then define the device implementation mapped to that UART
    mfc = new AlicatMFC(alicatUart);
    mfc->init();

    // Initialize Gauge UART and Device
    gaugeUart = new HardUart(uart0, P2_DI_PIN, P2_RO_PIN, 9600);
    // Use P2_TR_RE for the RTs pin
    gauge = new PfiefferGauge(gaugeUart, P2_TR_RE);
    gauge->init();
}

void SputteringManager::update() 
{
    if (mfc) 
    {
        mfc->update();
    }
    if (gauge)
    {
        gauge->update();
    }
    // Add logic here to manage other devices and control system states
}

void SputteringManager::sendTestMessage()
{
    if (gauge) 
    {
        gauge->sendMessage("0020074002=?107\r");
    }
    else
    {
        printf("Error: Gauge is null\n");
    }
}
#include "SputteringManager.h"
#include "picoDefinitions.h"
#include <stdio.h>

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
    alicatUart = new HardUart(uart0, ALICAT_1_TX, ALICAT_1_RX, 9600);
    
    // Then define the device implementation mapped to that UART
    mfc = new AlicatMFC(alicatUart);
    mfc->init();

    // Initialize Gauge UART and Device
    gaugeUart = new HardUart(uart1, P1_DI_PIN, P1_RO_PIN, 9600);
    gauge = new PfiefferGauge(gaugeUart);
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
    if (mfc) 
    {
        mfc->sendMessage("A\r");
    }
    else
    {
        printf("Error: MFC is null\n");
    }

    if (gauge) 
    {
        // TODO: format your message here
        gauge->sendMessage("0010074002=?106\r"); 
    }
    else
    {
        printf("Error: Gauge is null\n");
    }
}
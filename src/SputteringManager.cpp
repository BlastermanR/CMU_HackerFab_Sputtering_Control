#include "SputteringManager.h"
#include "PIO_UART.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <stdio.h>

SputteringManager::SputteringManager()
    : alicatUart(nullptr), alicatDevice(nullptr), mfc(nullptr), gaugeUart(nullptr), gaugeDevice(nullptr),
      gauge(nullptr), pumpUart(nullptr), pumpDevice(nullptr), pump(nullptr)
{
    // Constructor
}

SputteringManager::~SputteringManager()
{
    if (mfc)
    {
        delete mfc;
    }
    if (alicatDevice)
    {
        delete alicatDevice;
    }
    if (alicatUart)
    {
        delete alicatUart;
    }
    if (gauge)
    {
        delete gauge;
    }
    if (gaugeDevice)
    {
        delete gaugeDevice;
    }
    if (gaugeUart)
    {
        delete gaugeUart;
    }
    if (pump)
    {
        delete pump;
    }
    if (pumpDevice)
    {
        delete pumpDevice;
    }
    if (pumpUart)
    {
        delete pumpUart;
    }
}

void SputteringManager::init()
{
    printf("Sputtering Manager: Initializing...\n");

    // Dynamically define the UART first
    alicatUart   = new PioUart(pio0, 0, 1, ALICAT_1_TX, ALICAT_1_RX, 9600);
    alicatDevice = new RS232Device(alicatUart);

    // Then define the device implementation mapped to that UART
    mfc = new AlicatMFC(alicatDevice);
    mfc->init();

    // Initialize Gauge UART and Device on uart0 (P2 Pins)
    gaugeUart   = new HardUart(uart0, P2_DI_PIN, P2_RO_PIN, 9600);
    gaugeDevice = new RS485Device(gaugeUart, P2_TR_RE);
    gauge       = new PfiefferGauge(gaugeDevice);
    gauge->init();

    // Initialize Pump UART and Device on uart1 (P1 Pins)
    pumpUart   = new HardUart(uart1, P1_DI_PIN, P1_RO_PIN, 9600);
    pumpDevice = new RS485Device(pumpUart, P1_TR_RE);
    pump       = new PfiefferPump(pumpDevice);
    pump->init();
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
    if (pump)
    {
        pump->update();
    }
    // Add logic here to manage other devices and control system states
}

void SputteringManager::sendTestMessage()
{
    /*
    if (mfc)
    {
        mfc->sendMessage("A\r");
    }
    else
    {
        printf("Error: MFC is null\n");
    }
    */

    if (gauge)
    {
        gauge->sendMessage("0020074002=?107\r");
    }
    else
    {
        printf("Error: Gauge is null\n");
    }

    if (pump)
    {
        // NOTHING
    }
    else
    {
        printf("Error: Pump is null\n");
    }
}
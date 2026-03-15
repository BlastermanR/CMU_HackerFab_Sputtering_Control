#ifndef SPUTTERING_MANAGER_H
#define SPUTTERING_MANAGER_H

#include "AlicatMFC.h"
#include "PfiefferGauge.h"
#include "PfiefferPump.h"
#include "HardwareUART.h"
#include "RS232Device.h"
#include "RS485Device.h"

/**
 * @brief Manages the operation of the Sputtering automation system.
 * 
 * Currently acts as a skeletal manager orchestrating the Alicat MFC 
 * alongside future devices (Pumps, Gauges, etc.).
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
class SputteringManager {
private:
    IUart* alicatUart;
    RS232Device* alicatDevice;
    AlicatMFC* mfc;
    
    IUart* gaugeUart;
    RS485Device* gaugeDevice;
    PfiefferGauge* gauge;

    IUart* pumpUart;
    RS485Device* pumpDevice;
    PfiefferPump* pump;

public:
    /**
     * @brief Constructor
     */
    SputteringManager();

    /**
     * @brief Destructor
     */
    ~SputteringManager();

    /**
     * @brief Initialize the sputtering manager and its devices.
     */
    void init();

    /**
     * @brief Routine update loop. Meant to be called in the main infinite loop.
     * Updates states of all managed devices.
     */
    void update();


    /**
     * @brief Debug function to send a test message to the Alicat MFC.
     */
    void sendTestMessage();

};

#endif // SPUTTERING_MANAGER_H

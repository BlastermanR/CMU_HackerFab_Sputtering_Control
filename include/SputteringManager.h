#ifndef SPUTTERING_MANAGER_H
#define SPUTTERING_MANAGER_H

#include "AlicatMFC.h"
#include "PfiefferGauge.h"
#include "HardwareUART.h"

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
    HardUart* alicatUart;
    AlicatMFC* mfc;
    
    HardUart* gaugeUart;
    PfiefferGauge* gauge;

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

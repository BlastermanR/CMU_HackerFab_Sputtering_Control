#ifndef SPUTTERING_MANAGER_H
#define SPUTTERING_MANAGER_H

#include "AlicatMFC.h"
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
};

#endif // SPUTTERING_MANAGER_H

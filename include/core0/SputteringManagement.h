/**
 * SputteringManagement.h
 * @brief Management namespace for the Sputtering automation system.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/2/26
 */
#ifndef SPUTTERING_MANAGEMENT_H
#define SPUTTERING_MANAGEMENT_H

namespace SputteringManagement
{
    /**
     * @brief Continually updates all hardware devices and synchronizes telemetry.
     */
    void controlLoop();

    /**
     * @brief Performs a controlled shutdown by ramping down MFCs and disabling the pump.
     */
    void executeNormalShutdown();

    /**
     * @brief Immediately halts all operations during an error state.
     */
    void executeEmergencyShutdown();

    /**
     * @brief Manually polls all devices and updates shared telemetry memory.
     */
    void executePollDevices();
}

#endif // SPUTTERING_MANAGEMENT_H

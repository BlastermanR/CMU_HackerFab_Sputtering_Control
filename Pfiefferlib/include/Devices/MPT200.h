/**
 * @file MPT200.h
 * @brief Parameter definitions and device class for the Pfeiffer Vacuum MPT 200 / RPT 200 AR Gauge.
 *
 * Source:
 * https://www.idealvac.com/files/manuals/Pfeiffer_DigiLine_MPT_200_Digital_Pirani_and_Cold_Cathode_Gauge_Operating_Instructions.pdf
 *
 * @author Ryan Massie (rmassie)
 * @date 3/25/26
 */

#pragma once

#include "PfiefferDevice.h"
#include <cstdint>

namespace Pfieffer
{

/**
 * @brief Enum representing the MPT200 parameter numbers for easy, type-safe reference.
 */
enum class MPT200Cmd : uint16_t
{
    // Control Commands
    SensorOnOff      = 41,  //!< Sensor on/off
    SwitchMode       = 49,  //!< Switch mode
    PressureSetPoint = 741, //!< Pressure set point

    // Status / Value Requests
    Error                 = 303, //!< Actual error code
    SoftwareVersion       = 312, //!< Software version
    ComponentName         = 349, //!< Component name
    Pressure              = 740, //!< Actual pressure reading
    CorrectionPirani      = 742, //!< Correction value (Pirani)
    CorrectionColdCathode = 743  //!< Correction value (Cold cathode)
};

/**
 * @brief MPT200 / RPT200 Gauge implementation inheriting from the PfiefferDevice CRTP template.
 */
class MPT200 : public PfiefferDevice<MPT200>
{
  public:
    explicit MPT200(uint8_t address = 1) : PfiefferDevice<MPT200>(address) {}

    /**
     * @brief Constexpr array mapping all known MPT200 Parameters to definitions.
     */
    static constexpr PfiefferParamDef DICT[] = {
        // {ParamID, "Name", DataType, AccessType, MinVal, MaxVal, DefaultVal, NonVolatile}

        // Settings
        {41, "SensorOnOff", 6, AccessType::READ_WRITE, 0, 1, 0, false},
        {49, "SwitchMode", 7, AccessType::READ_WRITE, 0, 999, 0, true},
        {741, "PressureSetPoint", 7, AccessType::WRITE_ONLY, 0, 999, 0, false},
        {742, "CorrectionPirani", 2, AccessType::READ_WRITE, 20, 800, 100, true},
        {743, "CorrectionColdCathode", 2, AccessType::READ_WRITE, 20, 800, 100, true},

        // Status Readings
        {303, "Error", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {312, "SoftwareVersion", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {349, "ComponentName", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {740, "Pressure", 10, AccessType::READ_WRITE, 1e-10, 1000, 0, false}};

    /**
     * @brief Inherited CRTP static callback to fetch a parameter bound array entry.
     */
    static const PfiefferParamDef *getParamDef(uint16_t parameterNumber)
    {
        for (const auto &def : DICT)
        {
            if (def.number == parameterNumber)
            {
                return &def;
            }
        }
        return nullptr;
    }
};

} // namespace Pfieffer

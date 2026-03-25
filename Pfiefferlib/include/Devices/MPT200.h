/**
 * @file MPT200.h
 * @brief Parameter definitions and device class for the Pfeiffer Vacuum MPT 200 / RPT 200 AR Gauge.
 */

#pragma once

#include <cstdint>
#include "PfiefferDevice.h"

namespace Pfieffer {

/**
 * @brief Enum representing the MPT200 parameter numbers for easy, type-safe reference.
 */
enum class MPT200Cmd : uint16_t {
    // Control Commands
    SetPoint1 = 730,
    SetPoint2 = 732,
    SetPoint3 = 734,
    SetPoint4 = 736,
    FilterTime = 737,
    Emission = 738,    //!< Filament emission switch
    Degas = 739,       //!< Degas function

    // Status / Value Requests
    Pressure = 740,    //!< Actual pressure reading
    Error = 741,       //!< Error code / status
    HwVersion = 742,   //!< Hardware version
    FwVersion = 743,   //!< Firmware version
    DeviceName = 744   //!< Name of the device
};

/**
 * @brief MPT200 / RPT200 Gauge implementation inheriting from the PfiefferDevice CRTP template.
 */
class MPT200 : public PfiefferDevice<MPT200> {
public:
    explicit MPT200(uint8_t address = 1) : PfiefferDevice<MPT200>(address) {}

    /**
     * @brief Constexpr array mapping all known MPT200 Parameters to definitions.
     */
    static constexpr PfiefferParamDef DICT[] = {
        // {ParamID, "Name", DataType, AccessType, MinVal, MaxVal, DefaultVal, NonVolatile}
        
        // Settings 
        {730, "SetPoint1", 1, AccessType::READ_WRITE, 0, 999999, 0, true},
        {732, "SetPoint2", 1, AccessType::READ_WRITE, 0, 999999, 0, true},
        {734, "SetPoint3", 1, AccessType::READ_WRITE, 0, 999999, 0, true},
        {736, "SetPoint4", 1, AccessType::READ_WRITE, 0, 999999, 0, true},
        {737, "FilterTime", 1, AccessType::READ_WRITE, 0, 999999, 0, true},
        {738, "Emission", 0, AccessType::READ_WRITE, 0, 1, 0, false},
        {739, "Degas", 0, AccessType::READ_WRITE, 0, 1, 0, false},

        // Status Readings 
        {740, "Pressure", 7, AccessType::READ_ONLY, 1e-10, 1000, 0, false},
        {741, "Error", 4, AccessType::READ_ONLY, 0, 999999, 0, false},
        {742, "HwVersion", 4, AccessType::READ_ONLY, 0, 999999, 0, false},
        {743, "FwVersion", 4, AccessType::READ_ONLY, 0, 999999, 0, false},
        {744, "DeviceName", 4, AccessType::READ_ONLY, 0, 999999, 0, false}
    };

    /**
     * @brief Inherited CRTP static callback to fetch a parameter bound array entry.
     */
    static const PfiefferParamDef* getParamDef(uint16_t parameterNumber) {
        for (const auto& def : DICT) {
            if (def.number == parameterNumber) {
                return &def;
            }
        }
        return nullptr;
    }
};

} // namespace Pfieffer

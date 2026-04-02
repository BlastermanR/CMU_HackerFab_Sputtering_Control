/**
 * @file PfeifferDevice.h
 * @brief CRTP Template base class for Pfeiffer Vacuum devices.
 *
 * Provides a standardized template constraint over Pfeiffer commands
 * by forcing type safety, bounds checking, and structured property definitions.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/25/26
 */

#pragma once

#include "PfeifferLib.h"
#include <cstdint>
#include <cstdio>
#include <string>

namespace Pfeiffer
{

/**
 * @brief Defines the read/write permissions for a specific device parameter.
 */
enum class AccessType
{
    READ_ONLY,  //!< Parameter can only be read from the device.
    WRITE_ONLY, //!< Parameter can only be written to the device.
    READ_WRITE  //!< Parameter supports both read and write operations.
};

/**
 * @brief Generic struct representing the constraints and properties of a device parameter.
 */
struct PfeifferParamDef
{
    uint16_t    number;      //!< Device-specific parameter number identifier.
    const char *name;        //!< Short string descriptor of the parameter.
    uint8_t     dataType;    //!< Formatting type of the parameter within the protocol.
    AccessType  access;      //!< Read/write access permissions.
    double      min;         //!< Minimum permissible limit for value input.
    double      max;         //!< Maximum permissible limit for value input.
    double      defaultVal;  //!< Default factory setting.
    bool        nonVolatile; //!< True if parameter survives power cycles.
};

/**
 * @brief CRTP Template representing a Generic Pfeiffer Device.
 *
 * This base class forces device derivatives (like TC110DriveUnit) to statically bind
 * their own parameter definition properties (`getParamDef`) without virtual overhead.
 *
 * @tparam Derived The specific device implementation class.
 */
template <typename Derived> class PfeifferDevice
{
  protected:
    uint8_t m_address;

  public:
    explicit PfeifferDevice(uint8_t address = 1) : m_address(address) {}

    /**
     * @brief Creates a validated Read Command targeting this device.
     *
     * @param paramNumber The parameter to query.
     * @param outCmd Reference to a PfeifferCommand struct to be populated.
     * @return true if successful, false if access denied or parameter missing.
     */
    bool createReadCommand(uint16_t paramNumber, PfeifferCommand &outCmd) const
    {
        const PfeifferParamDef *def = Derived::getParamDef(paramNumber);
        if (!def || def->access == AccessType::WRITE_ONLY)
        {
            return false;
        }

        char addrStr[4];
        snprintf(addrStr, sizeof(addrStr), "%03d", m_address);
        char paramStr[4];
        snprintf(paramStr, sizeof(paramStr), "%03d", paramNumber);

        outCmd.address  = addrStr;
        outCmd.action   = READ_PARAMETER;
        outCmd.paramNum = paramStr;
        outCmd.data     = QUERY_DATA_STR; // Pfeiffer protocol requests usually have this data for queries

        return true;
    }

    /**
     * @brief Creates a validated bounds-checked Write Command targeting this device.
     *
     * @param paramNumber The parameter to overwrite.
     * @param value The numeric value to send.
     * @param outCmd Reference to a PfeifferCommand struct to be populated.
     * @return true on success, false if access denied, out of bounds, or parameter missing.
     */
    bool createWriteCommand(uint16_t paramNumber, double value, PfeifferCommand &outCmd) const
    {
        const PfeifferParamDef *def = Derived::getParamDef(paramNumber);
        if (!def || def->access == AccessType::READ_ONLY)
        {
            return false;
        }

        if (value < def->min || value > def->max)
        {
            return false; // Bounds validation failed
        }

        char addrStr[4];
        snprintf(addrStr, sizeof(addrStr), "%03d", m_address);
        char paramStr[4];
        snprintf(paramStr, sizeof(paramStr), "%03d", paramNumber);

        // Typical Write command data is a 6-digit zero-padded string
        // Data Types might vary in protocol size in the future but 0-padded int works for primary limits
        char dataStr[32];
        snprintf(dataStr, sizeof(dataStr), "%06d", static_cast<int>(value));

        outCmd.address  = addrStr;
        outCmd.action   = DATA_RESPONSE; // Write action block
        outCmd.paramNum = paramStr;
        outCmd.data     = dataStr;

        return true;
    }

    /**
     * @brief Returns the RS-485 device address.
     * @return The device address byte.
     */
    uint8_t getAddress() const { return m_address; }

    /**
     * @brief Sets the RS-485 device address.
     * @param addr The new device address byte.
     */
    void setAddress(uint8_t addr) { m_address = addr; }
};

} // namespace Pfeiffer

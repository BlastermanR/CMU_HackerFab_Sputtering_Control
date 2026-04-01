#ifndef ALICAT_LIB_H
#define ALICAT_LIB_H
/**
 * @file AlicatLib.h
 * @brief Utility library for formatting and parsing Alicat Mass Flow Controller commands.
 *
 * Defines the AlicatCommand and AlicatResponse structures and provides
 * static methods for protocol serialization and string parsing.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/25/26
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include "AlicatErrors.h"
#include "AlicatGases.h"

/**
 * Primary Alicat Command Actions
 * https://documents.alicat.com/Alicat-Serial-Primer.pdf
 */
inline const std::string ALICAT_POLL            = "";     // Obtain a data frame
inline const std::string ALICAT_SET_GAS         = "G";    // Change current active gas
inline const std::string ALICAT_ACTIVE_GAS      = "GS";   // Query, change, or save active gas
inline const std::string ALICAT_AVAILABLE_GASES = "??G*"; // Obtain a list of available gases installed on the device
inline const std::string ALICAT_CHANGE_SETPOINT = "S";    // Change the current setpoint
inline const std::string ALICAT_QUERY_CHANGE_SETPOINT  = "LS"; // Query, change, or save the current setpoint
inline const std::string ALICAT_TARE_FLOW              = "V";  // Create a zero flow reference point
inline const std::string ALICAT_TARE_ABSOLUTE_PRESSURE = "PC"; // Create a zero absolute pressure reference point
inline const std::string ALICAT_TARE_GAUGE_PRESSURE =
    "P";                                                 // Create a zero gauge or differential pressure reference point
inline const std::string ALICAT_START_STREAMING = "@ @"; // Start streaming data from a device
inline const std::string ALICAT_STOP_STREAMING  = "@@";  // Stop streaming data from a device

/**
 * @brief Represents a command sent to an Alicat MFC
 */
struct AlicatCommand
{
    char        id{'A'};    // Device ID (typically 'A' - 'Z')
    std::string action{""}; // Command letter (e.g. "" for Poll, "S" for New Setpoint in newer firmware)
    std::string data{""};   // Value attached to command (e.g. "5.0")
};

/**
 * @brief Represents a parsed data frame from an Alicat MFC
 *
 * Standard Alicat format returns:
 * ID | Absolute Pressure | Temperature | Volumetric Flow | Mass Flow | Setpoint | Gas
 */
struct AlicatDataFrame
{
    char        id{'A'};
    double      pressure{0.0};
    double      temperature{0.0};
    double      volumetricFlow{0.0};
    double      massFlow{0.0};
    double      setpoint{0.0};
    std::string              gasType{""};
    std::vector<std::string> statusCodes{}; // Status/error codes present in the frame (e.g. "MOV", "POV")
};

class AlicatLib
{
  public:
    /**
     * @brief Format an AlicatCommand struct into a properly structured command
     * string to send to the device.
     * @param command Pointer to the AlicatCommand struct to format.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is constructed, false otherwise.
     * @return The formatted command string ending in a carriage return.
     */
    static std::string formatCommand(const AlicatCommand *command, bool *valid = nullptr)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to formatCommand.\n");
            if (valid)
                *valid = false;
            return "";
        }

        // Compile string (e.g., 'A', 'S', '5.0' -> "AS5.0\r")
        // Polls are simply the ID: (e.g., 'A' -> "A\r")
        std::string formattedCommand = std::string(1, command->id) + command->action + command->data + "\r";

        if (valid)
            *valid = true;

        return formattedCommand;
    }

    /**
     * @brief Parse a response string received from the device and populate an
     * AlicatDataFrame struct with the extracted fields.
     * @param response The raw response string received from the device.
     * @param frame Pointer to the AlicatDataFrame struct to populate.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * response is valid and parsed successfully, false otherwise.
     */
    /**
     * @brief Validate that a gas ID exists in the Alicat gas table.
     * @param gasId Gas number to check (use ALICAT_GAS_* defines).
     * @return true if the ID is a known Alicat gas, false otherwise.
     */
    static bool isValidGasId(uint8_t gasId)
    {
        return lookupAlicatGasById(gasId) != nullptr;
    }

    /**
     * @brief Get the short name for a gas ID (e.g. "Ar" for Argon).
     * @param gasId Gas number to look up.
     * @return The short name C-string, or nullptr if the ID is unknown.
     */
    static const char *getGasShortName(uint8_t gasId)
    {
        const AlicatGasDef *gas = lookupAlicatGasById(gasId);
        return gas ? gas->shortName : nullptr;
    }

    /**
     * @brief Get the long name for a gas ID (e.g. "Argon").
     * @param gasId Gas number to look up.
     * @return The long name C-string, or nullptr if the ID is unknown.
     */
    static const char *getGasLongName(uint8_t gasId)
    {
        const AlicatGasDef *gas = lookupAlicatGasById(gasId);
        return gas ? gas->longName : nullptr;
    }

    /**
     * @brief Check whether a string is a known Alicat status code.
     * @param code Null-terminated string to check (e.g. "MOV").
     * @return true if the string matches a known status code.
     */
    static bool isStatusCode(const char *code)
    {
        return lookupAlicatStatusByCode(code) != nullptr;
    }

    /**
     * @brief Get the human-readable description for a status code.
     * @param code The 3-character status code (e.g. "MOV").
     * @return Description C-string, or nullptr if not recognised.
     */
    static const char *getStatusDescription(const char *code)
    {
        const AlicatStatusDef *s = lookupAlicatStatusByCode(code);
        return s ? s->description : nullptr;
    }

    static void parseResponse(const std::string &response, AlicatDataFrame *frame, bool *valid = nullptr)
    {
        if (valid)
            *valid = false; // Default to false until fully validated

        if (frame == nullptr)
        {
            printf("Error: Null frame pointer passed to parseResponse.\n");
            return;
        }

        if (response.empty())
        {
            printf("Error: Response string is empty.\n");
            return;
        }

        // Tokenize strings by spaces
        std::vector<std::string> tokens;
        std::stringstream        ss(response);
        std::string              token;
        while (ss >> token)
        {
            tokens.push_back(token);
        }

        // An Alicat data frame usually has 7 delimited components (if standard gas and no extra fields):
        // 0: ID (A)
        // 1: Pressure
        // 2: Temperature
        // 3: Volumetric Flow
        // 4: Mass Flow
        // 5: Setpoint
        // 6: Gas Type
        if (tokens.size() < 7)
        {
            printf("Error: Response does not contain enough tokens to form a complete frame.\nReceived: %s\n",
                   response.c_str());
            return;
        }

        // Helper lambda for safe double conversion without exceptions
        auto safeStrtod = [](const std::string &str, double &outVal) -> bool
        {
            char *endPtr;
            outVal = std::strtod(str.c_str(), &endPtr);
            return endPtr != str.c_str(); // Returns true if extraction occurred
        };

        frame->id = tokens[0][0]; // Take first character of the ID token

        bool parseSuccess = true;
        parseSuccess &= safeStrtod(tokens[1], frame->pressure);
        parseSuccess &= safeStrtod(tokens[2], frame->temperature);
        parseSuccess &= safeStrtod(tokens[3], frame->volumetricFlow);
        parseSuccess &= safeStrtod(tokens[4], frame->massFlow);
        parseSuccess &= safeStrtod(tokens[5], frame->setpoint);
        frame->gasType = tokens[6];

        // Collect any status/error codes from trailing tokens (tokens[7+]).
        // Only tokens matching known Alicat status codes are collected;
        // unrecognised trailing tokens are silently ignored.
        frame->statusCodes.clear();
        for (std::size_t ti = 7; ti < tokens.size(); ++ti)
        {
            if (lookupAlicatStatusByCode(tokens[ti].c_str()) != nullptr)
                frame->statusCodes.push_back(tokens[ti]);
        }

        if (parseSuccess)
        {
            if (valid)
                *valid = true; // Fully validated
        }
        else
        {
            printf("Error: Failed to parse float values from response.\nReceived: %s\n", response.c_str());
            if (valid)
                *valid = false;
        }
    }
};

#endif // ALICAT_LIB_H
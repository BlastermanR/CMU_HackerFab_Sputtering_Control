#ifndef PFEIFFERLIB_H
#define PFEIFFERLIB_H
/**
 * @file PfeifferLib.h
 * @brief Utility library for formatting and parsing Pfeiffer Vacuum protocol commands.
 *
 * Defines the PfeifferCommand structure and provides static methods for
 * checksum calculation and protocol serialization.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>

/**
 * API Variables
 */
#define PFEIFFER_ADDRESS_LENGTH 3
#define PFEIFFER_ACTION_LENGTH 2
#define PFEIFFER_PARAMNUM_LENGTH 3
#define PFEIFFER_DATALEN_LENGTH 2
#define PFEIFFER_CHECKSUM_LENGTH 3
#define PFEIFFER_LOGIC_SIZE                                                                                            \
    (PFEIFFER_ADDRESS_LENGTH + PFEIFFER_ACTION_LENGTH + PFEIFFER_PARAMNUM_LENGTH + PFEIFFER_DATALEN_LENGTH +           \
     PFEIFFER_CHECKSUM_LENGTH + 1) // Minimum size of a valid command/response (without data)
inline const std::string READ_PARAMETER = "00";
inline const std::string DATA_RESPONSE  = "10";
inline const std::string ERROR_RESPONSE = "20";
inline const std::string QUERY_DATA_STR = "=?";

/**
 * @brief Pfeiffer protocol data type identifiers.
 * See PFEIFFER_RS485_PROTOCOL.md Section 7.1 for full definitions.
 */
namespace PfeifferDataType
{
    constexpr uint8_t BOOLEAN      = 0;  //!< 6 chars: "000000" (false) / "111111" (true)
    constexpr uint8_t U_INTEGER    = 1;  //!< 6 chars: zero-padded positive integer
    constexpr uint8_t U_REAL       = 2;  //!< 6 chars: fixed-point divide 100 (e.g. "001571" = 15.71)
    constexpr uint8_t STRING_SHORT = 4;  //!< 6 chars: ASCII string
    constexpr uint8_t BOOLEAN_NEW  = 6;  //!< 7 chars: "ON     " / "OFF    "
    constexpr uint8_t U_SHORT_INT  = 7;  //!< 3 chars: zero-padded short positive integer
    constexpr uint8_t U_EXPO_NEW   = 10; //!< 6 chars: encoded pressure (device-specific lookup)
    constexpr uint8_t STRING_LONG  = 11; //!< 16 chars: ASCII string
} // namespace PfeifferDataType

/**
 * PfeifferCommand Struct
 */
struct PfeifferCommand
{
    std::string address{""};  // 3 characters
    std::string action{""};   // 2 characters
    std::string paramNum{""}; // 3 characters
    std::string data{""};     // Data field (size can be adjusted as needed)
    std::string dataLen{""};  // 3 characters
    std::string checksum{""}; // Checksum (2 characters)
};

class PfeifferLib
{
    /**
     * @brief Calculate the checksum for a given command based on its fields.
     * The checksum is the sum of the ASCII values of all characters in the
     * command fields modulo 256.
     * @param command Pointer to the PfeifferCommand struct to calculate the
     * checksum for.
     * @return The calculated checksum as an unsigned integer.
     */
    static unsigned int calculateChecksum(const PfeifferCommand *command)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to calculateChecksum.\n");
            return 0;
        }

        unsigned int sum        = 0;
        auto         add_to_sum = [&sum](const std::string &str)
        {
            for (unsigned char c : str)
            {
                sum += c;
            }
        };

        add_to_sum(command->address);
        add_to_sum(command->action);
        add_to_sum(command->paramNum);
        add_to_sum(command->dataLen);
        add_to_sum(command->data);

        return sum % 256; // Modulo 256, formatted as 3 digits
    }

  public:
    /**
     * @brief Format a PfeifferCommand struct into a properly structured command
     * string to send to the device. This function calculates the checksum and
     * data length fields automatically based on the command contents.
     * @param command Pointer to the PfeifferCommand struct to format.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     * @return The formatted command string.
     */
    static std::string formatCommand(PfeifferCommand *command, bool *valid = nullptr)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to formatCommand.\n");
            if (valid)
                *valid = false;
            return "";
        }

        // Calculate data length (2 digits zero-padded)
        char lenBuf[3];
        snprintf(lenBuf, sizeof(lenBuf), "%02u", (unsigned int)command->data.size());
        command->dataLen = lenBuf;

        // Calculate the checksum (modulo 256, 3 digits zero-padded)
        char csumBuf[4];
        snprintf(csumBuf, sizeof(csumBuf), "%03u", calculateChecksum(command));
        command->checksum = csumBuf;

        // Compile string
        std::string formattedCommand = command->address + command->action + command->paramNum + command->dataLen +
                                       command->data + command->checksum + "\r"; // Append carriage return

        if (valid)
            *valid = true;

        return formattedCommand;
    }

    /**
     * @brief Parse a response string received from the device and populate a
     * PfeifferCommand struct with the extracted fields. This function also
     * performs validation checks on the response format and checksum.
     * @param response The raw response string received from the device.
     * @param command Pointer to the PfeifferCommand struct to populate with the
     * parsed data
     * @param valid Optional pointer to a boolean that will be set to true if the
     * response is valid and parsed successfully, false otherwise.
     */
    static void decryptResponse(std::string response, PfeifferCommand *command, bool *valid = nullptr)
    {
        if (valid)
            *valid = false; // Default to false until fully validated

        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to decryptResponse.\n");
            return;
        }

        if (response.size() < PFEIFFER_LOGIC_SIZE)
        {
            printf("Error: Response string too short to parse.\nReceived response: %s\n", response.c_str());
            return;
        }

        unsigned int offset = 0;

        // Helper lambda to extract strings, validate characters, and advance offset
        auto extract = [&](unsigned int len, std::string &target, bool allowPrintable = false) -> bool
        {
            if (offset + len > response.size())
                return false;

            target = response.substr(offset, len);
            for (unsigned char c : target)
            {
                if (allowPrintable ? !std::isprint(c) : !std::isdigit(c))
                    return false;
            }
            offset += len;
            return true;
        };

        auto parseError = [&]()
        { printf("Error: Invalid character or formatting in response.\nReceived response: %s\n", response.c_str()); };

        // Extract fixed-length header fields
        if (!extract(PFEIFFER_ADDRESS_LENGTH, command->address) || !extract(PFEIFFER_ACTION_LENGTH, command->action) ||
            !extract(PFEIFFER_PARAMNUM_LENGTH, command->paramNum) ||
            !extract(PFEIFFER_DATALEN_LENGTH, command->dataLen))
        {
            return parseError();
        }

        // Extract dynamic-length data field
        const unsigned int EXPECTED_DATA_LENGTH = std::stoi(command->dataLen);
        if (!extract(EXPECTED_DATA_LENGTH, command->data, true))
        {
            return parseError();
        }

        // Extract checksum
        if (!extract(PFEIFFER_CHECKSUM_LENGTH, command->checksum))
        {
            return parseError();
        }

        // Validate checksum
        if (std::stoi(command->checksum) != calculateChecksum(command))
        {
            printf("Error: Checksum mismatch in response.\nReceived response: %s\n", response.c_str());
            return;
        }

        // Note: the carriage return is used as the frame delimiter by the serial
        // layer (onDataReceived) and is stripped before the message reaches here.
        // Do not validate it here.

        if (valid)
            *valid = true; // Fully validated
    }

    /**
     * @brief Utility function to check if a given command is a read command based
     * on its action field.
     * @param command Pointer to the PfeifferCommand struct to check.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     * @return True if the command is a read command, false otherwise.
     */
    static bool isRead(PfeifferCommand *command, bool *valid = nullptr)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to isRead.\n");
            if (valid)
                *valid = false;
            return false;
        }

        return (command->action == READ_PARAMETER) ? true : false;
    }

    /**
     * @brief Utility function to print the contents of a PfeifferCommand struct
     * for debugging purposes.
     * @param command Pointer to the PfeifferCommand struct to print.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     */
    static void printCommand(PfeifferCommand *command, bool *valid = nullptr)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to printCommand.\n");
            if (valid)
                *valid = false;
            return;
        }

        printf("Address: %s\n", command->address.c_str());
        printf("Action: %s\n", command->action.c_str());
        printf("Parameter Number: %s\n", command->paramNum.c_str());
        printf("Data Length: %s\n", command->dataLen.c_str());
        printf("Data: %s\n", command->data.c_str());
        printf("Checksum: %s\n", command->checksum.c_str());
    }

    /**
     * @brief Returns the expected data field length in characters for a given Pfeiffer data type.
     * @param dataType The Pfeiffer data type identifier (see PfeifferDataType namespace).
     * @return The number of data characters transmitted on the wire.
     */
    static uint8_t getDataLength(uint8_t dataType)
    {
        switch (dataType)
        {
        case PfeifferDataType::BOOLEAN:      return 6;
        case PfeifferDataType::U_INTEGER:    return 6;
        case PfeifferDataType::U_REAL:       return 6;
        case PfeifferDataType::STRING_SHORT: return 6;
        case PfeifferDataType::BOOLEAN_NEW:  return 7;
        case PfeifferDataType::U_SHORT_INT:  return 3;
        case PfeifferDataType::U_EXPO_NEW:   return 6;
        case PfeifferDataType::STRING_LONG:  return 16;
        default:                             return 6;
        }
    }

    /**
     * @brief Encode a numeric value into its Pfeiffer wire-format string based on data type.
     * @param dataType The Pfeiffer data type identifier.
     * @param value The numeric value to encode.
     * @return The encoded data string, or empty string if the type does not support numeric encoding.
     */
    static std::string encodeValue(uint8_t dataType, double value)
    {
        char buf[32];
        switch (dataType)
        {
        case PfeifferDataType::BOOLEAN:
            return (value != 0.0) ? "111111" : "000000";

        case PfeifferDataType::U_INTEGER:
            snprintf(buf, sizeof(buf), "%06d", static_cast<int>(value));
            return buf;

        case PfeifferDataType::U_REAL:
            snprintf(buf, sizeof(buf), "%06d", static_cast<int>(value * 100.0));
            return buf;

        case PfeifferDataType::BOOLEAN_NEW:
            return (value != 0.0) ? "ON     " : "OFF    ";

        case PfeifferDataType::U_SHORT_INT:
            snprintf(buf, sizeof(buf), "%03d", static_cast<int>(value));
            return buf;

        default:
            return "";
        }
    }

    /**
     * @brief Decode a Pfeiffer wire-format data string back to a numeric value based on data type.
     * @param dataType The Pfeiffer data type identifier.
     * @param data The raw data string from the device response.
     * @return The decoded numeric value. Returns 0.0 for string-only types (4, 11).
     */
    static double decodeValue(uint8_t dataType, const std::string &data)
    {
        switch (dataType)
        {
        case PfeifferDataType::BOOLEAN:
            return (data == "111111") ? 1.0 : 0.0;

        case PfeifferDataType::U_INTEGER:
            return static_cast<double>(std::strtol(data.c_str(), nullptr, 10));

        case PfeifferDataType::U_REAL:
            return static_cast<double>(std::strtol(data.c_str(), nullptr, 10)) / 100.0;

        case PfeifferDataType::BOOLEAN_NEW:
            return (data.size() >= 2 && data[0] == 'O' && data[1] == 'N') ? 1.0 : 0.0;

        case PfeifferDataType::U_SHORT_INT:
            return static_cast<double>(std::strtol(data.c_str(), nullptr, 10));

        case PfeifferDataType::U_EXPO_NEW:
        {
            // Pressure encoding: first 4 chars = mantissa, last 2 = exponent index.
            // Common convention: value = mantissa * 10^(expIndex - 20).
            // Verify against your specific hardware documentation.
            if (data.size() >= 6)
            {
                double mantissa = static_cast<double>(std::strtol(data.substr(0, 4).c_str(), nullptr, 10));
                int    expIndex = static_cast<int>(std::strtol(data.substr(4, 2).c_str(), nullptr, 10));
                return mantissa * std::pow(10.0, expIndex - 20);
            }
            return 0.0;
        }

        default:
            return static_cast<double>(std::strtol(data.c_str(), nullptr, 10));
        }
    }
};

#endif // PFEIFFERLIB_H
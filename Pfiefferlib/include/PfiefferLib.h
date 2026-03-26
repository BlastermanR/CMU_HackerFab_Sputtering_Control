#ifndef PFIEFFERLIB_H
#define PFIEFFERLIB_H
/**
 * @file PfiefferLib.h
 * @brief Utility library for formatting and parsing Pfeiffer Vacuum protocol commands.
 *
 * Defines the PfiefferCommand structure and provides static methods for
 * checksum calculation and protocol serialization.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <cstdint>
#include <string>

/**
 * API Variables
 */
#define PFIEFFER_ADDRESS_LENGTH 3
#define PFIEFFER_ACTION_LENGTH 2
#define PFIEFFER_PARAMNUM_LENGTH 3
#define PFIEFFER_DATALEN_LENGTH 3
#define PFIEFFER_CHECKSUM_LENGTH 2
#define PFIEFFER_LOGIC_SIZE                                                                                            \
    (PFIEFFER_ADDRESS_LENGTH + PFIEFFER_ACTION_LENGTH + PFIEFFER_PARAMNUM_LENGTH + PFIEFFER_DATALEN_LENGTH +           \
     1) // Minimum size of a valid command/response (without data + carraige
        // return)
inline const std::string READ_PARAMETER     = "00";
inline const std::string DESCRIBE_PARAMETER = "10";

/**
 * PfiefferCommand Struct
 */
struct PfiefferCommand
{
    std::string address{""};  // 3 characters
    std::string action{""};   // 2 characters
    std::string paramNum{""}; // 3 characters
    std::string data{""};     // Data field (size can be adjusted as needed)
    std::string dataLen{""};  // 3 characters
    std::string checksum{""}; // Checksum (2 characters)
};

class PfieifferLib
{
    /**
     * @brief Calculate the checksum for a given command based on its fields.
     * The checksum is the sum of the ASCII values of all characters in the
     * command fields modulo 256.
     * @param command Pointer to the PfiefferCommand struct to calculate the
     * checksum for.
     * @return The calculated checksum as an unsigned integer.
     */
    static unsigned int calculateChecksum(const PfiefferCommand *command)
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

        return sum % 256; // Modulo 256 to fit in 2 characters
    }

  public:
    /**
     * @brief Format a PfiefferCommand struct into a properly structured command
     * string to send to the device. This function calculates the checksum and
     * data length fields automatically based on the command contents.
     * @param command Pointer to the PfiefferCommand struct to format.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     * @return The formatted command string.
     */
    static std::string formatCommand(PfiefferCommand *command, bool *valid = nullptr)
    {
        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to formatCommand.\n");
            if (valid)
                *valid = false;
            return "";
        }

        // Calculate the checksum
        command->checksum = std::to_string(calculateChecksum(command));

        // Calculate data length
        command->dataLen = std::to_string(command->data.size());

        // Compile string
        std::string formattedCommand = command->address + command->action + command->paramNum + command->dataLen +
                                       command->data + command->checksum + "\r"; // Append carriage return

        return formattedCommand;
    }

    /**
     * @brief Parse a response string received from the device and populate a
     * PfiefferCommand struct with the extracted fields. This function also
     * performs validation checks on the response format and checksum.
     * @param response The raw response string received from the device.
     * @param command Pointer to the PfiefferCommand struct to populate with the
     * parsed data
     * @param valid Optional pointer to a boolean that will be set to true if the
     * response is valid and parsed successfully, false otherwise.
     */
    static void decryptResponse(std::string response, PfiefferCommand *command, bool *valid = nullptr)
    {
        if (valid)
            *valid = false; // Default to false until fully validated

        if (command == nullptr)
        {
            printf("Error: Null command pointer passed to decryptResponse.\n");
            return;
        }

        if (response.size() < PFIEFFER_LOGIC_SIZE)
        {
            printf("Error: Response string too short to parse.\nReceived response: %s\n", response.c_str());
            return;
        }

        unsigned int offset = 0;

        // Helper lambda to extract strings, validate characters, and advance offset
        auto extract = [&](unsigned int len, std::string &target, bool isAlphaNum = false) -> bool
        {
            if (offset + len > response.size())
                return false;

            target = response.substr(offset, len);
            for (char c : target)
            {
                if (isAlphaNum ? !std::isalnum(c) : !std::isdigit(c))
                    return false;
            }
            offset += len;
            return true;
        };

        auto parseError = [&]()
        { printf("Error: Invalid character or formatting in response.\nReceived response: %s\n", response.c_str()); };

        // Extract fixed-length header fields
        if (!extract(PFIEFFER_ADDRESS_LENGTH, command->address) || !extract(PFIEFFER_ACTION_LENGTH, command->action) ||
            !extract(PFIEFFER_PARAMNUM_LENGTH, command->paramNum) ||
            !extract(PFIEFFER_DATALEN_LENGTH, command->dataLen))
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
        if (!extract(PFIEFFER_CHECKSUM_LENGTH, command->checksum))
        {
            return parseError();
        }

        // Validate checksum
        if (std::stoi(command->checksum) != calculateChecksum(command))
        {
            printf("Error: Checksum mismatch in response.\nReceived response: %s\n", response.c_str());
            return;
        }

        // Validate carriage return
        if (response.back() != '\r')
        {
            printf("Error: Missing carriage return at end of response.\nReceived response: %s\n", response.c_str());
            return;
        }

        if (valid)
            *valid = true; // Fully validated
    }

    /**
     * @brief Utility function to check if a given command is a read command based
     * on its action field.
     * @param command Pointer to the PfiefferCommand struct to check.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     * @return True if the command is a read command, false otherwise.
     */
    static bool isRead(PfiefferCommand *command, bool *valid = nullptr)
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
     * @brief Utility function to print the contents of a PfiefferCommand struct
     * for debugging purposes.
     * @param command Pointer to the PfiefferCommand struct to print.
     * @param valid Optional pointer to a boolean that will be set to true if the
     * command is valid, false otherwise.
     */
    static void printCommand(PfiefferCommand *command, bool *valid = nullptr)
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
};

#endif // PFIEFFERLIB_H
#ifndef PFIEFFERLIB_H
#define PFIEFFERLIB_H
/**
 * Defines a library of usefule objects and functions for communicating with Pfieffer Vacuum pumps.
 */

#include <string>
#include <cstdint>

// Command Values
// TODO

// API Variables
#define PFIEFFER_ADDRESS_LENGTH 3
#define PFIEFFER_ACTION_LENGTH 2
#define PFIEFFER_PARAMNUM_LENGTH 3
#define PFIEFFER_DATALEN_LENGTH 3
#define PFIEFFER_CHECKSUM_LENGTH 2
#define PFIEFFER_LOGIC_SIZE (PFIEFFER_ADDRESS_LENGTH + PFIEFFER_ACTION_LENGTH + PFIEFFER_PARAMNUM_LENGTH + PFIEFFER_DATALEN_LENGTH + 1) // Minimum size of a valid command/response (without data + carraige return)

inline const std::string READ_PARAMETER = "00";
inline const std::string DESCRIBE_PARAMETER = "10";

struct PfiefferCommand 
{
    std::string address{""};  // 3 characters
    std::string action{""};   // 2 characters
    std::string paramNum{""}; // 3 characters
    std::string data{""};     // Data field (size can be adjusted as needed)
    std::string dataLen{""};  // 3 characters
    std::string checksum{""}; // Checksum (2 characters)
};

class PfieifferLib {
    /**
     * @brief Calculate the checksum for a given command based on its fields.
     * The checksum is the sum of the ASCII values of all characters in the command fields modulo 256.
     * @param command Pointer to the PfiefferCommand struct to calculate the checksum for.
     * @return The calculated checksum as an unsigned integer.
     */
    static unsigned int calculateChecksum(PfiefferCommand* command)
    {
        if(command == nullptr) 
        {
            printf("Error: Null command pointer passed to calculateChecksum.\n");
            return 0;
        }

        unsigned int sum = 0;

        for (int i = 0; i < command->address.size(); i++) {
            sum += command->address[i];
        }

        for (int i = 0; i < command->action.size(); i++) {
            sum += command->action[i];
        }

        for (int i = 0; i < command->paramNum.size(); i++) {
            sum += command->paramNum[i];
        }

        for (int i = 0; i < command->dataLen.size(); i++) {
            sum += command->dataLen[i];
        }

        for (int i = 0; i < command->data.size(); i++) {
            sum += command->data[i];
        }

        return sum % (UINT8_MAX + 1); // Modulo 256 to fit in 2 characters
    }

    public:
    /**
     * @brief Format a PfiefferCommand struct into a properly structured command string to send to the device.
     * This function calculates the checksum and data length fields automatically based on the command contents.
     * @param command Pointer to the PfiefferCommand struct to format.
     * @param valid Optional pointer to a boolean that will be set to true if the command is valid, false otherwise.
     * @return The formatted command string.
     */
    static std::string formatCommand(PfiefferCommand* command, bool* valid = nullptr)
    {
        if(command == nullptr) 
        {
            printf("Error: Null command pointer passed to formatCommand.\n");
            if (valid) *valid = false;
            return "";
        }

        // Calculate the checksum
        command->checksum = std::to_string(calculateChecksum(command));

        // Calculate data length
        command->dataLen = std::to_string(command->data.size());

        // Compile string
        std::string formattedCommand = command->address + command->action + command->paramNum + command->dataLen + command->data + command->checksum + "\r"; // Append carriage return

        return formattedCommand;
    }

    /**
     * @brief Parse a response string received from the device and populate a PfiefferCommand struct with the extracted fields.
     * This function also performs validation checks on the response format and checksum.
     * @param response The raw response string received from the device.
     * @param command Pointer to the PfiefferCommand struct to populate with the parsed data
     * @param valid Optional pointer to a boolean that will be set to true if the response is valid and parsed successfully, false otherwise.
     */
    static void decryptResponse(std::string response, PfiefferCommand* command, bool* valid = nullptr)
    {
        if(command == nullptr) 
        {
            printf("Error: Null command pointer passed to decryptResponse.\n");
            if (valid) *valid = false;
            return;
        }

        if (response.size() < PFIEFFER_LOGIC_SIZE)
        {
            printf("Error: Response string too short to parse.\n");
            printf("Received response: %s\n", response.c_str());
            if (valid) *valid = false;
            return;
        }

        unsigned int offset = 0;

        // Read address
        for (int i = 0; i < PFIEFFER_ADDRESS_LENGTH; i++) 
        {
            if (!std::isdigit(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }
            command->address += response[i];
        }
        offset += PFIEFFER_ADDRESS_LENGTH;

        // Read action
        for (int i = 0; i < PFIEFFER_ACTION_LENGTH; i++) 
        {
            if (!std::isdigit(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            } 
            command->action += response[offset + i];
        }
        offset += PFIEFFER_ACTION_LENGTH;

        // Read parameter number
        for (int i = 0; i < PFIEFFER_PARAMNUM_LENGTH; i++) 
        {
            if (!std::isdigit(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }
            command->paramNum += response[offset + i];
        }
        offset += PFIEFFER_PARAMNUM_LENGTH;

        // Read data length
        for (int i = 0; i < PFIEFFER_DATALEN_LENGTH; i++) 
        {
            if (!std::isdigit(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }
            command->dataLen += response[offset + i];
        }
        offset += PFIEFFER_DATALEN_LENGTH;

        // Read data based on data length
        const unsigned int EXPECTED_DATA_LENGTH = std::stoi(command->dataLen);
        for (int i = 0; i < EXPECTED_DATA_LENGTH; i++) 
        {
            if (!std::isalnum(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }
            command->data += response[offset + i];
        }
        offset += EXPECTED_DATA_LENGTH;

        // Read checksum
        for (int i = 0; i < PFIEFFER_CHECKSUM_LENGTH; i++) 
        {
            if (!std::isdigit(response[i]))
            {
                printf("Error: Invalid character in address field of response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }

            command->checksum += response[offset + i];
            
            if (std::stoi(command->checksum) != calculateChecksum(command))
            {
                printf("Error: Checksum mismatch in response.\n");
                printf("Received response: %s\n", response.c_str());
                if (valid) *valid = false;
                return;
            }
        }

        // Check for carriage return at the end
        if (response.back() != '\r')
        {
            printf("Error: Missing carriage return at end of response.\n");
            printf("Received response: %s\n", response.c_str());
            if (valid) *valid = false;
            return;
        }
    }

    /**
     * @brief Utility function to check if a given command is a read command based on its action field.
     * @param command Pointer to the PfiefferCommand struct to check.
     * @param valid Optional pointer to a boolean that will be set to true if the command is valid, false otherwise.
     * @return True if the command is a read command, false otherwise.
     */
    static bool isRead(PfiefferCommand* command, bool* valid = nullptr)
    {
        if(command == nullptr) 
        {
            printf("Error: Null command pointer passed to isRead.\n");
            if (valid) *valid = false;
            return false;
        }

        return (command->action == READ_PARAMETER) ? true : false;
    }

    /**
     * @brief Utility function to print the contents of a PfiefferCommand struct for debugging purposes.
     * @param command Pointer to the PfiefferCommand struct to print.
     * @param valid Optional pointer to a boolean that will be set to true if the command is valid, false otherwise.
     */
    static void printCommand(PfiefferCommand* command, bool* valid = nullptr)
    {
        if(command == nullptr) 
        {
            printf("Error: Null command pointer passed to printCommand.\n");
            if (valid) *valid = false;
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
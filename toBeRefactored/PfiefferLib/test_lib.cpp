#include <iostream>
#include <cstring>
#include "ArduinoPfiefferVacuum.h"  // Include your header file

// Mock Arduino-specific functions
unsigned long millis() {
    return static_cast<unsigned long>(clock());
}

// Mock SoftwareSerial (since we're not using actual serial communication)
class SoftwareSerial {
public:
    void println(const char* msg) {
        std::cout << "Mock Serial Output: " << msg << std::endl;
    }
    bool available() { return false; }  // Simulate no data available
    char read() { return '\0'; }  // Simulate no incoming data
};

int main() {
    // Instantiate class with a test address
    ArduinoPfiefferVacuum pump("ADR");

    // Test command formatting
    ASCII_char command = pump.pfieffer_command_format("00", "123", "02", "=?");
    if (command) {
        std::cout << "Formatted Command: " << command << std::endl;
        pump.free_message(command);  // Free allocated memory
    } else {
        std::cerr << "Error: Command formatting failed!" << std::endl;
    }

    // Test response decryption
    ASCII_char response = strdup("ADR12302OK");  // Simulate a pump response
    ASCII_char* parsed = pump.decrypt_response(response);
    if (parsed) {
        std::cout << "Decrypted Address: " << parsed[0] << std::endl;
        std::cout << "Decrypted Param: " << parsed[1] << std::endl;
        std::cout << "Decrypted Data Len: " << parsed[2] << std::endl;
        std::cout << "Decrypted Data: " << parsed[3] << std::endl;
        pump.free_decrypt_response(parsed);
    } else {
        std::cerr << "Error: Response decryption failed!" << std::endl;
    }
    free((void*)response);

    // Test checksum generation
    ASCII_char checksum = pump.get_check_sum((char*)"ADR12302OK", 10);
    if (checksum) {
        std::cout << "Checksum: " << checksum << std::endl;
        pump.free_check_sum(checksum);
    } else {
        std::cerr << "Error: Checksum calculation failed!" << std::endl;
    }

    // Test data request
    ASCII_char data_request = pump.data_request("397");
    if (data_request) {
        std::cout << "Data Request: " << data_request << std::endl;
        pump.free_message(data_request);
    }

    return 0;
}

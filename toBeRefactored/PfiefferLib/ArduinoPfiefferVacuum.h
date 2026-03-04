/*
  ArduinoPfiefferVacuum.h - Library for formatting Pfieffer Vacuum pump commands.
  Created by Shayaan Gandhi, February 16, 2024.
*/

#ifndef ArduinoPfiefferVacuum_h
#define ArduinoPfiefferVacuum_h

//#include <Arduino.h>
//#include <SoftwareSerial.h>

typedef const char *ASCII_char;  // Move typedef outside the class

class ArduinoPfiefferVacuum
{
    public:
        ArduinoPfiefferVacuum(ASCII_char address);
        ~ArduinoPfiefferVacuum();  // Destructor to free memory
        ASCII_char pfieffer_command_format(ASCII_char write, ASCII_char param_nums, ASCII_char data_len, ASCII_char data);
        ASCII_char* decrypt_response(ASCII_char response);
        ASCII_char get_check_sum(char *without_checksum, uint8_t len);
        ASCII_char data_request(ASCII_char param_num);
        ASCII_char control_request(ASCII_char param_num, ASCII_char data);
        void free_message(ASCII_char message);
        void free_check_sum(ASCII_char message);
        void free_decrypt_response(ASCII_char* response_array);
        char _carriage_return;
    private:
        ASCII_char _address;  // Store a copy of address
};

#endif

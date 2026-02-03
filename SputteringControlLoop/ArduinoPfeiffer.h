#pragma once

/*
  ArduinoPfeiffer.h - Library for formatting Pfeiffer Vacuum pump commands.
  Created by Shayaan Gandhi, February 16, 2025.
  Modified by Kyle Minihan, February 3, 2026.
*/

#include <stdint.h>

// Define ASCII_char as a pointer to a character string
typedef char* ASCII_char;

// Class definition for handling Pfeiffer vacuum command formatting
class ArduinoPfeiffer
{
public:
    // Constructor: takes device address as input
    ArduinoPfeiffer(ASCII_char address);

    // Destructor
    ~ArduinoPfeiffer();

    // Formats a complete command string using Pfeiffer syntax
    ASCII_char pfeiffer_command_format(ASCII_char write, ASCII_char param_nums, ASCII_char data_len, ASCII_char data);

    // Computes the 3-digit checksum string from a given message
    ASCII_char get_check_sum(char *without_checksum, uint8_t len);

    // Returns a formatted data request command string
    ASCII_char data_request(ASCII_char param_num);

    // Returns a formatted control command string
    ASCII_char control_request(ASCII_char param_num, ASCII_char data);

    // Frees dynamically allocated memory created by this class
    void free_message(ASCII_char message);

    // Carriage return character (ASCII code 13)
    char _carriage_return;

private:
    // Stored device address (used as prefix in commands)
    ASCII_char _address;
};

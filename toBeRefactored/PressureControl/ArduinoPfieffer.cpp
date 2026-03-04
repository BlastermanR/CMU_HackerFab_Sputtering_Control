/*
  ArduinoPfieffer.cpp - Library for formatting Pfieffer Vacuum pump commands.
  Created by Shayaan Gandhi, February 16, 2025.
*/

#include "ArduinoPfieffer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

// Constructor: initializes address and sets carriage return character
ArduinoPfieffer::ArduinoPfieffer(ASCII_char address)
{
    _address = address;
    _carriage_return = 13; // ASCII code for carriage return '\r'
}

// Destructor: nothing to free since no dynamic members are owned by this class
ArduinoPfieffer::~ArduinoPfieffer()
{
    // No dynamic members in class itself; nothing to free
}

// Formats a complete Pfieffer command including checksum and carriage return
ASCII_char ArduinoPfieffer::pfieffer_command_format(ASCII_char write, ASCII_char param_nums, ASCII_char data_len, ASCII_char data)
{
    // Calculate total length needed for the buffer:
    // command components + checksum (up to 3 digits) + carriage return + null terminator
    uint32_t comm_len = strlen(_address) + strlen(write) + strlen(param_nums)
                      + strlen(data_len) + strlen(data) + 3 + 1 + 1;

    // Allocate buffer for the command string
    ASCII_char buf = (ASCII_char)malloc(comm_len * sizeof(char));
    if (!buf) return NULL; // Return NULL if malloc fails

    // Build the message by concatenating components
    buf[0] = '\0'; // Start with an empty string
    strcat(buf, _address);
    strcat(buf, write);
    strcat(buf, param_nums);
    strcat(buf, data_len);
    strcat(buf, data);

    // Compute checksum and append it
    ASCII_char check_sum = get_check_sum(buf, strlen(buf));
    strcat(buf, check_sum);      // Append checksum
    strcat(buf, "\r");           // Append carriage return at the end
    free(check_sum);             // Free temporary checksum string

    return buf; // Return the final command string
}

// Computes a 3-digit ASCII checksum for a given string
ASCII_char ArduinoPfieffer::get_check_sum(char *without_checksum, uint8_t len)
{
    uint8_t check_sum = 0;

    // Sum ASCII values of each character in the message
    for (uint8_t i = 0; i < len; i++) {
        check_sum += without_checksum[i];
    }

    // Allocate 4 characters (3 digits + null terminator)
    ASCII_char sum_char = (ASCII_char)malloc(4 * sizeof(char));
    if (!sum_char) {
        return NULL; // Return NULL if malloc fails
    }

    // Format the checksum as a 3-digit string
    sprintf(sum_char, "%03d", check_sum);
    return sum_char; // Return the formatted checksum string
}

// Creates a properly formatted data request command
ASCII_char ArduinoPfieffer::data_request(ASCII_char param_num)
{
    // "00" is the read mode for a data request, "02" is fixed length for "=?"
    return pfieffer_command_format("00", param_num, "02", "=?");
}

// Creates a control command with given parameter number and data
ASCII_char ArduinoPfieffer::control_request(ASCII_char param_num, ASCII_char data)
{
    // Compute length of data and convert it to 2-digit ASCII string
    uint8_t data_len = strlen(data);
    char data_len_char[3]; // 2 digits + null terminator
    sprintf(data_len_char, "%02d", data_len);

    // Use the write mode "10" to send the control command
    return pfieffer_command_format("10", param_num, data_len_char, data);
}

// Frees memory allocated for any ASCII_char message created by this class
void ArduinoPfieffer::free_message(ASCII_char message)
{
    if (message) {
        free(message); // Only free if non-null
    }
}
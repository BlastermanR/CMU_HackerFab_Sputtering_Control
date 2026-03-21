/*
  ArduinoPfiefferVacuum.cpp - Library for formatting Pfieffer Vacuum pump
  commands. Created by Shayaan Gandhi, Febrauary 16, 2024.
*/

#include <ArduinoPfiefferVacuum.h>
// #include <SoftwareSerial.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char *ASCII_char;

ArduinoPfiefferVacuum::ArduinoPfiefferVacuum(ASCII_char address)
{
    _address         = address;
    _carriage_return = 13;
}
// Need to implement error codes for most processes

ASCII_char ArduinoPfiefferVacuum::pfieffer_command_format(ASCII_char write, ASCII_char param_nums, ASCII_char data_len,
                                                          ASCII_char data)
{
    // Calculate the total required buffer length
    uint32_t comm_len = strlen(_address) + strlen(write) + strlen(param_nums) + strlen(data_len) + strlen(data) + 4;

    // Allocate memory (including space for null terminator)
    ASCII_char buf = (ASCII_char)malloc((comm_len + 1) * sizeof(char));
    if (!buf)
    {
        return NULL; // Handle memory allocation failure
    }

    // Initialize buffer as an empty string
    buf[0] = '\0';

    // Concatenate each string sequentially
    strcat(buf, _address);
    strcat(buf, write);
    strcat(buf, param_nums);
    strcat(buf, data_len);
    strcat(buf, data);

    // Compute checksum based on the existing content
    ASCII_char check_sum = get_check_sum(buf, strlen(buf));

    // Append checksum and carriage return at the correct positions
    size_t len = strlen(buf);
    strcat(buf, check_sum);
    buf[len + 1] = _carriage_return;
    buf[len + 2] = '\0'; // Null terminator
    free(check_sum);
    return buf; // Caller must free this memory when done
}

ASCII_char *ArduinoPfiefferVacuum::decrypt_response(ASCII_char response)
{
    // Allocate memory for each extracted component
    ASCII_char address  = (ASCII_char)malloc(4 * sizeof(char));
    ASCII_char param    = (ASCII_char)malloc(4 * sizeof(char));
    ASCII_char data_len = (ASCII_char)malloc(3 * sizeof(char));

    if (!address || !param || !data_len)
    {
        free(address);
        free(param);
        free(data_len);
        return NULL; // Handle allocation failure
    }

    // Get address
    strncpy(address, response, 3);
    address[3] = '\0';

    // Get param
    strncpy(param, response + 3, 3);
    param[3] = '\0';

    // Get data_len
    strncpy(data_len, response + 6, 2);
    data_len[2] = '\0';

    // Convert data_len to an integer
    uint8_t data_len_num = atoi(data_len);

    // Allocate memory for the data based on extracted length
    ASCII_char data = (ASCII_char)malloc((data_len_num + 1) * sizeof(char));
    if (!data)
    {
        free(address);
        free(param);
        free(data_len);
        return NULL; // Handle allocation failure
    }

    // Get data
    strncpy(data, response + 8, data_len_num);
    data[data_len_num] = '\0'; // Ensure null termination

    // Allocate memory for the return array (array of pointers)
    ASCII_char *val_array = (ASCII_char *)malloc(4 * sizeof(ASCII_char));
    if (!val_array)
    {
        free(address);
        free(param);
        free(data_len);
        free(data);
        return NULL; // Handle allocation failure
    }

    // Store extracted values in the dynamically allocated array
    val_array[0] = address;
    val_array[1] = param;
    val_array[2] = data_len;
    val_array[3] = data;

    return val_array; // Caller must free the memory
}

ASCII_char ArduinoPfiefferVacuum::get_check_sum(char *without_checksum, uint8_t len)
{
    uint8_t check_sum = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        check_sum += without_checksum[i];
    }

    // Allocate memory for the checksum string
    ASCII_char sum_char = (ASCII_char)malloc(4 * sizeof(char)); // Max 3 digits + NULL terminator
    if (!sum_char)
    {
        return NULL; // Handle allocation failure
    }

    sprintf(sum_char, "%d", check_sum); // Convert checksum to string

    return sum_char; // Caller must free() this memory
}

ASCII_char ArduinoPfiefferVacuum::data_request(ASCII_char param_num)
{
    return pfieffer_command_format("00", param_num, "02", "=?");
}

ASCII_char ArduinoPfiefferVacuum::control_request(ASCII_char param_num, ASCII_char data)
{
    uint8_t data_len = strlen(data);

    char data_len_char[3];
    sprintf(data_len_char, "%d", data_len);

    return pfieffer_command_format("10", param_num, data_len_char, data);
}

void ArduinoPfiefferVacuum::free_message(ASCII_char message)
{
    free(message);
    return;
}

void ArduinoPfiefferVacuum::free_check_sum(ASCII_char check_sum)
{
    free(check_sum);
    return;
}

void ArduinoPfiefferVacuum::free_decrypt_response(ASCII_char *response_array)
{
    if (!response_array)
        return; // Prevent dereferencing NULL

    // Free each dynamically allocated string if it's not NULL
    for (int i = 0; i < 4; i++)
    {
        if (response_array[i])
        {
            free(response_array[i]);  // Cast to void* before freeing
            response_array[i] = NULL; // Prevent dangling pointer issues
        }
    }

    // Free the array itself
    free(response_array);  // Cast to void* before freeing
    response_array = NULL; // Avoid use-after-free errors
}

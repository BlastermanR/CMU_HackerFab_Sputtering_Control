#pragma once
#include <SoftwareSerial.h>
#include "ArduinoPfeiffer.h"
#include <Arduino.h>


typedef struct rs485_response
{
    char param[4] = {0};
    char value[7] = {0};
};


/**
 * @class RS485Device
 * @brief Controls an RS485 serial communication device with hardware flow control.
 * 
 * This class manages a software serial interface with RS485 transceiver control pins.
 * It provides methods to switch between read and write modes via DE (Driver Enable) and
 * RE (Receiver Enable) pins.
 */
class RS485Device {
public:

    /**
     * @brief Constructs an RS485Device instance.
     * 
     * @param rxPin Pin number for serial receive (connected to RX of SoftwareSerial).
     * @param txPin Pin number for serial transmit (connected to TX of SoftwareSerial).
     * @param dePin Pin number for DE (Driver Enable) control signal.
     * @param rePin Pin number for RE (Receiver Enable) control signal.
     */
    RS485Device(uint8_t rxPin, uint8_t txPin,
                uint8_t dePin, uint8_t rePin)
      : _ss(rxPin, txPin),
        _dePin(dePin),
        _rePin(rePin) {}

    /** @brief Initializes the RS485 device and serial communication. */
    void begin(long baud) {
        pinMode(_dePin, OUTPUT);
        pinMode(_rePin, OUTPUT);
        setReadMode();
        _ss.begin(baud);
    }

    /** @brief Sets the device to read mode. */
    void setReadMode() {
        digitalWrite(_dePin, LOW);
        digitalWrite(_rePin, LOW);
    }

    /** @brief Sets the device to write mode. */
    void setWriteMode() {
        digitalWrite(_dePin, HIGH);
        digitalWrite(_rePin, HIGH);
    }


    /** @brief Provides access to the underlying SoftwareSerial port. */
    SoftwareSerial& port() { return _ss; }

    rs485_response readRS485Reply();
    void sendRS485Command (ASCII_char cmd);

private:
    SoftwareSerial _ss;
    uint8_t _dePin;
    uint8_t _rePin;

    #define SENTENCE_SIZE 128
    char sentence[SENTENCE_SIZE];
    int sentenceIndex = 0;
    const char endChar = '\r';

    rs485_response parseRS485Response (char* msg);
};

#include "SerialDeviceBase.h"
#include <algorithm>

SerialDeviceBase::SerialDeviceBase(IUart* uartInstance) : uart(uartInstance), receiveIndex(0) {
    // Initialize the queue to hold elements of type SerialMessage
    queue_init(&msgQueue, sizeof(SerialMessage), SERIAL_QUEUE_SIZE);
    // Bind the callback
    uart->setCallback([this](char c) {
        this->onDataReceived(c);
    });
}

SerialDeviceBase::~SerialDeviceBase() {
    queue_free(&msgQueue);
}

void SerialDeviceBase::begin() {
    uart->begin();
}

bool SerialDeviceBase::hasMessage() {
    return !queue_is_empty(&msgQueue);
}

std::string SerialDeviceBase::popMessage() {
    SerialMessage msg;
    if (queue_try_remove(&msgQueue, &msg)) {
        return std::string(msg.data);
    }
    return "";
}

void SerialDeviceBase::onDataReceived(char c) {
    if (c == '\n' || c == '\r') {
        if (receiveIndex > 0) {
            SerialMessage msg;
            // Ensure null termination and avoid buffer overflow
            size_t copyLen = std::min((size_t)receiveIndex, sizeof(msg.data) - 1);
            std::memcpy(msg.data, receiveBuffer, copyLen);
            msg.data[copyLen] = '\0';
            
            queue_try_add(&msgQueue, &msg);
            receiveIndex = 0;
        }
    } else {
        if (receiveIndex < sizeof(receiveBuffer) - 1) {
            receiveBuffer[receiveIndex++] = c;
        }
    }
}

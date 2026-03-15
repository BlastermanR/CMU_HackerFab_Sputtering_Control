#include "SerialDeviceBase.h"
#include <algorithm>

SerialDeviceBase::SerialDeviceBase(IUart* uartInstance) : uart(uartInstance) {
    // Initialize the queue to hold 10 elements of type SerialMessage
    queue_init(&msgQueue, sizeof(SerialMessage), 10);
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

bool SerialDeviceBase::hasMessage() const {
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
        if (!receiveBuffer.empty()) {
            SerialMessage msg;
            // Ensure null termination and avoid buffer overflow
            size_t copyLen = std::min(receiveBuffer.length(), sizeof(msg.data) - 1);
            std::memcpy(msg.data, receiveBuffer.c_str(), copyLen);
            msg.data[copyLen] = '\0';
            
            queue_try_add(&msgQueue, &msg);
            receiveBuffer.clear();
        }
    } else {
        receiveBuffer.push_back(c);
    }
}

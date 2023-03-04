#pragma once

#include "device_interface.hpp"

#include <chrono>
#include <thread>

namespace framework64::asset_transfer {

class NullDevice : DeviceInterface {
public:
    size_t send_message_time_milliseconds = 16;

    virtual bool sendMessage(uint8_t header, void* data, size_t size) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(send_message_time_milliseconds));
    }

    const char* identifier() const override {
        return "null";
    }
};

}
#pragma once

#include <cstdint>

namespace framework64::asset_transfer {

class DeviceInterface {
public:
    virtual bool sendMessage(uint8_t header, void* data, size_t size) = 0;
    virtual const char* identifier() const = 0;
};

}
#pragma once

#include "device_interface.hpp"

#include <string>

namespace framework64::asset_transfer{

class N64Device : public DeviceInterface {
public:
    bool initialize(std::string const & rom_path);

public:
    virtual bool sendMessage(uint8_t header, void* data, size_t size) override;
    virtual const char* identifier() const override;

private:
    std::string _rom_path;
};

}
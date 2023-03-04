#pragma once

#include "device_interface.hpp"

#include <string>

namespace framework64::asset_transfer{

class N64Device : public DeviceInterface {
public:
    bool initialize(std::string const & rom_path);

public:
    virtual bool transferStaticMesh(std::string const & path) override;
    virtual const char* identifier() const override;

private:
    std::string _rom_path;
};

}
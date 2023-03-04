#pragma once

#include <cstdint>
#include <string>

namespace framework64::asset_transfer {

class DeviceInterface {
public:
    virtual bool transferStaticMesh(std::string const & path) = 0;
    virtual const char* identifier() const = 0;
};

}
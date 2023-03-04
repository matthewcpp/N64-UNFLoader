#pragma once

#include "converter.hpp"
#include "device_interface.hpp"
#include "settings.hpp"

#include <uuid.h>

namespace framework64::asset_transfer {

class Processor {
public:
    Processor(Settings const & settings, DeviceInterface & di);

    bool processStaticMesh(std::string const & file);

private:
    Converter converter;
    DeviceInterface & device_interface;
    uuids::uuid_random_generator uuid_generator;

    std::filesystem::path temp_dir;
};

}
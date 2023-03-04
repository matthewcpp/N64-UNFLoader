#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "n64_device.hpp"
#include "null_device.hpp"
#include "settings.hpp"
#include "processor.hpp"
#include "web_service.hpp"


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: ./fw64_asset_transfer path/to/settings.json" << std::endl;
        return 1;
    }

    framework64::asset_transfer::Settings settings;
    if (!settings.loadSettingsFile(argv[1])) {
        return 1;
    }

#if 1
    framework64::asset_transfer::N64Device device;
    device.initialize(settings.rom_file.string());
#else
    framework64::asset_transfer::NullDevice device;
#endif

    framework64::asset_transfer::Processor processor(settings, device);

    framework64::asset_transfer::WebService web_service(processor);
    web_service.run();

    return 0;
}

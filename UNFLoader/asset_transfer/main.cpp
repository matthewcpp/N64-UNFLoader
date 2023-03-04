#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "n64_device.hpp"
#include "settings.hpp"
#include "processor.hpp"
#include "web_service.hpp"

int main(int argc, char** argv) {
    framework64::asset_transfer::Settings settings;
    std::string settings_path = (argc >= 2) ? argv[1] : "settings.json";

    if (!settings.loadSettingsFile(settings_path))
        return 1;

    framework64::asset_transfer::N64Device device;
    device.initialize(settings.rom_file.string());

    framework64::asset_transfer::Processor processor(settings, device);

    framework64::asset_transfer::WebService web_service(processor);
    web_service.run();

    return 0;
}

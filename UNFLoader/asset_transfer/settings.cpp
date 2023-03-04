#include "settings.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <fstream>

namespace framework64::asset_transfer {

bool Settings::loadSettingsFile(std::string const & settings_file_path) {

    std::ifstream f(settings_file_path);

    if (!f) {
        std::cout << "Unable to open settings file: " << settings_file_path << std::endl;
        return false;
    }

    json data = json::parse(f);

    auto not_found = data.end();
    if (data.find("framework64_directory") != not_found) {
        auto framework64_directory_str = data["framework64_directory"].get<std::string>();

        if (!std::filesystem::is_directory(framework64_directory_str)) {
            std::cout << "Invalid framework64 directory: " << framework64_directory_str << std::endl;
            return false;
        }

        framework64_directory = framework64_directory_str;
    }

    if (data.find("rom_file") != not_found) {
        auto rom_file_str = data["rom_file"].get<std::string>();

        if (!std::filesystem::is_regular_file(rom_file_str)) {
            std::cout << "Invalid rom file path: " << rom_file_str;
            return false;
        }

        rom_file = rom_file_str;
    }

    if (data.find("platform") != not_found) {
        platform = data["platform"].get<std::string>();
    }

    return true;
}


}
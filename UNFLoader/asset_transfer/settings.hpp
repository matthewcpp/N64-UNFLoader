#pragma once

#include <filesystem>
#include <string>


namespace framework64::asset_transfer {

class Settings {
public:
    std::filesystem::path framework64_directory;
    std::filesystem::path rom_file;
    std::string platform;

    bool loadSettingsFile(std::string const & settings_file_path);


};

}
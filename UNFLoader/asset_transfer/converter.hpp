#pragma once

#include "settings.hpp"

#include <filesystem>
#include <string>

namespace framework64::asset_transfer {

class Converter {
public:
    Converter(Settings const & s);

    std::string convertStaticMesh(std::filesystem::path const & path, std::filesystem::path const & scratch_dir);

private:
    Settings const & settings;
    std::string pipeline_script_path;
};

}
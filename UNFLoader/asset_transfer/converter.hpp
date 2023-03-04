#pragma once

#include <filesystem>
#include <string>

namespace framework64::asset_transfer {

class Converter {
public:

    std::string convertFile(std::filesystem::path const & path, std::filesystem::path const & scratch_dir, std::string const & platform);
};

}
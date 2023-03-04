#include "converter.hpp"

#include "process.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <fstream>

namespace framework64::asset_transfer {

Converter::Converter(Settings const & s): settings(s) {
    auto pipeline_script_file = settings.framework64_directory / "scripts" / "RunPipeline.js";
    pipeline_script_path = pipeline_script_file.string();
}

std::string write_asset_json(std::filesystem::path const & path, std::filesystem::path const & scratch_dir) {
    json assets_json;
    assets_json["meshes"] = json::array();
    assets_json["meshes"].push_back({{"src", path.filename().string()}});

    std::filesystem::path assets_json_path = scratch_dir / "asset.json";
    std::ofstream assets_json_file(assets_json_path);
    assets_json_file << std::setw(4) << assets_json << std::endl;

    return assets_json_path.string();
}

std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot); 
}

std::string Converter::convertStaticMesh(std::filesystem::path const & path, std::filesystem::path const & scratch_dir) {
    std::string assets_json_path = write_asset_json(path, scratch_dir);
    std::string assets_dir = path.parent_path().string();
    std::string output_dir = scratch_dir.string();

    std::vector<std::string> command_args;
    command_args.push_back("node");
    command_args.push_back(pipeline_script_path);
    command_args.push_back(assets_json_path);
    command_args.push_back(assets_dir);
    command_args.push_back(output_dir);
    command_args.push_back(settings.platform);

    TinyProcessLib::Process(command_args, "", [](const char *bytes, size_t n){
        std::string stdout_str(bytes, n);
        std::cout << stdout_str;
    });

    std::string converted_file_name = remove_extension(path.filename().string()) + ".mesh";
    std::filesystem::path converted_file_path = scratch_dir / converted_file_name;

    return converted_file_path.string();
}

}
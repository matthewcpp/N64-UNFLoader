#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "process.hpp"
#include <uuid.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string rom_path = "D:/development/repos/framework64/build_n64/bin/data_link.z64";
std::filesystem::path fw64_directory = "D:/development/repos/framework64/";
std::filesystem::path working_dir = "D:/temp/fw64_convert";
std::string pipeline_script_path;
std::filesystem::path incomming_file_path = "D:/temp/fw64_incomming/mesh.gltf";


void send_file(std::string const & path_str);
uuids::uuid_random_generator init_uuid();
bool process_file(std::string const & path , std::string const & uuid);

void on_device_error(const char* error) {
    std::cout << "Fatal Error: " << error;
}

void on_sendrom(float percent) {
    if (percent == 1.0f) {
        std::cout << "Rom Uploaded.  Type .exit to terminate program." << std::endl;
    }
}

void on_device_message(const char* message) {
    std::cout << "Device message:" << message;
}

void process_incomming_messages() {
    while (device_get_pending()) {
        uint32_t info = device_begin_read();

        uint8_t command = (info >> 24) & 0xFF;
        uint32_t size = info & 0xFFFFFF;
        
        std::vector<char> buf(size, 0);
        device_read(buf.data(), size);
        device_end_read();

        if (command == DATATYPE_TEXT){
            std::string message(buf.data(), size);
            std::cout << "N64: " << message << std::endl;
        }
        else if (command == 3) {
            uint32_t big_endian;
            std::memcpy(&big_endian, buf.data(), sizeof(uint32_t));

            uint32_t little_endian = _byteswap_ulong(big_endian);
        }

    }
}

//TODO: add additional swapping intrinsics : https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c

struct BeginMessage {
    uint32_t file_name_size;
    uint32_t file_data_size;
    char file_name[32];

    BeginMessage(std::string const & name, uint32_t file_size) {
        std::memset(file_name, 0, sizeof(file_name));
        std::copy(name.begin(), name.end(), &file_name[0]);
        file_name_size = _byteswap_ulong(static_cast<uint32_t>(name.size()));
        file_data_size = _byteswap_ulong(file_size);
    }
};

#define DATA_MESSAGE_SIZE 1064
#define DATA_MESSAGE_PAYLOAD_SIZE (DATA_MESSAGE_SIZE - sizeof(uint32_t))
struct DataMessage {
    uint32_t payload_size;
    char payload[DATA_MESSAGE_PAYLOAD_SIZE];
};

void send_file(std::string const & path_str) {
    std::filesystem::path asset_path = path_str;

    auto file_name = asset_path.filename().string();
    auto file_size = static_cast<size_t>(std::filesystem::file_size(asset_path));

    std::cout << "BeginMessage: " << sizeof(BeginMessage) << " DataMessage: " << sizeof(DataMessage) << std::endl;
    std::cout << "Writing " << file_name << " (" << file_size << " bytes) to N64" << std::endl;

    std::ifstream file(asset_path, std::ios::binary);

    BeginMessage begin_message (file_name, file_size);
    DataMessage data_message;

    process_incomming_messages();
    
    device_senddata(1, reinterpret_cast<char*>(&begin_message), sizeof(BeginMessage));

    size_t data_sent = 0;
    while (data_sent < file_size) {
        size_t data_remaining = file_size - data_sent;
        size_t amount_to_send = data_remaining > DATA_MESSAGE_PAYLOAD_SIZE ? DATA_MESSAGE_PAYLOAD_SIZE : data_remaining;
        data_sent += amount_to_send;

        data_message.payload_size = _byteswap_ulong(amount_to_send);
        file.read(&data_message.payload[0], amount_to_send);

        device_senddata(2, reinterpret_cast<char*>(&data_message), sizeof(DataMessage));

    }

    std::cout << "Transfer Complete" << std::endl;
}

int main(int argc, char** argv) {
    auto uuid_generator = init_uuid();
    pipeline_script_path = (fw64_directory / "scripts" / "RunPipeline.js").string();

    device_sendrom_params_t sendrom_params;
    device_sendrom_params_init(&sendrom_params);
    device_set_fatal_error_callback(on_device_error);
    device_set_message_callback(on_device_message);
    device_set_sendrom_progress_callback(on_sendrom);
    device_find(CART_ANY);
    device_open();
    device_sendrom(rom_path.c_str(), &sendrom_params);
    std::cout << "ROM Uploaded to device" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

#if 0
    //process_incomming_messages();
    std::string asset_path_str;

    for (;;) {
        std::cout << "Enter asset path:" << std::endl;
        std::getline(std::cin, asset_path_str);

        if (asset_path_str == ".exit")
            break;        

        if (!std::filesystem::exists(asset_path_str)) {
            std::cout << "invalid path specified." << std::endl;
            continue;
        }

        // std::string file_path = "D:/development/repos/framework64/build_n64/bin/data_link/assets/controller_cube.mesh";

        //send_file(asset_path_str);
        uuids::uuid id = uuid_generator();
        auto result = process_file(asset_path_str, uuids::to_string(id));
        if (!result) {
            std::cout << "failed to process file" << std::endl;
        }
    }
#endif

for (;;) {
        if (std::filesystem::exists(incomming_file_path)) {
        uuids::uuid id = uuid_generator();
        auto result = process_file(incomming_file_path.string(), uuids::to_string(id));
        if (!result) {
            std::cout << "failed to process file" << std::endl;
        }

        std::filesystem::remove(incomming_file_path);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}


    return 0;
}

uuids::uuid_random_generator init_uuid() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};
    return gen;
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

std::string convert_file(std::filesystem::path const & path, std::filesystem::path const & scratch_dir) {
    std::string assets_json_path = write_asset_json(path, scratch_dir);
    std::string assets_dir = path.parent_path().string();
    std::string output_dir = scratch_dir.string();

    std::vector<std::string> command_args;
    command_args.push_back("node");
    command_args.push_back(pipeline_script_path);
    command_args.push_back(assets_json_path);
    command_args.push_back(assets_dir);
    command_args.push_back(output_dir);
    command_args.push_back("N64");

    TinyProcessLib::Process(command_args, "", [](const char *bytes, size_t n){
        std::string stdout_str(bytes, n);
        std::cout << stdout_str;
    });

    std::string converted_file_name = remove_extension(path.filename().string()) + ".mesh";
    std::filesystem::path converted_file_path = scratch_dir / converted_file_name;

    return converted_file_path.string();
}

bool process_file(std::string const & file , std::string const & uuid) {
    std::filesystem::path file_path = file;
    std::filesystem::path scratch_dir = working_dir / uuid;

    std::cout << "Processing file: " << file << std::endl;
    std::cout << "Scratch dir: " << scratch_dir.string() << std::endl;

    std::filesystem::create_directory(scratch_dir);
    auto converted_file_path = convert_file(file_path, scratch_dir);

    auto file_was_converted = std::filesystem::exists(converted_file_path);
    if (file_was_converted) {
        send_file(converted_file_path);
    }

    std::filesystem::remove_all(scratch_dir);

    return file_was_converted;
}
#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <restinio/all.hpp>

#include "process.hpp"
#include <uuid.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "n64_device.hpp"
#include "null_device.hpp"

std::string rom_path = "D:/development/repos/framework64/build_n64/bin/data_link.z64";
std::filesystem::path fw64_directory = "D:/development/repos/framework64/";
std::filesystem::path working_dir = "D:/temp/fw64_convert";
std::string pipeline_script_path;
std::filesystem::path incomming_file_path = "D:/temp/fw64_incomming/mesh.gltf";


void send_file(std::string const & path_str);
uuids::uuid_random_generator init_uuid();
bool process_file(std::string const & path , std::string const & uuid);

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

int main(int argc, char** argv) {
    auto uuid_generator = init_uuid();
    pipeline_script_path = (fw64_directory / "scripts" / "RunPipeline.js").string();

#if 1
    framework64::asset_transfer::N64Device device;
    device.initialize(rom_path);
#else
    framework64::asset_transfer::NullDevice device;
#endif


using namespace restinio;
    auto router = std::make_unique<router::express_router_t<>>();
    router->http_post(
            "/display_mesh",
            [&uuid_generator](auto req, auto params) {
                json req_json = json::parse(req->body());
                auto const & file_path = req_json["file"].get<std::string>();
                std::cout << "Processing Mesh: " << file_path;
                uuids::uuid id = uuid_generator();
                auto result = process_file(file_path, uuids::to_string(id));
                return req->create_response()
                        .set_body(file_path)
                        .done();
            });

    router->non_matched_request_handler(
            [](auto req){
                return req->create_response(restinio::status_not_found()).connection_close().done();
            });

    // Launching a server with custom traits.
    struct my_server_traits : public default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };

    restinio::run(
            restinio::on_this_thread<my_server_traits>()
                    .address("localhost")
                    .port(8080)
                    .request_handler(std::move(router)));

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

void send_file(std::string const & path_str) {
    std::filesystem::path asset_path = path_str;

    auto file_name = asset_path.filename().string();
    auto file_size = static_cast<size_t>(std::filesystem::file_size(asset_path));

    std::cout << "BeginMessage: " << sizeof(BeginMessage) << " DataMessage: " << sizeof(DataMessage) << std::endl;
    std::cout << "Writing " << file_name << " (" << file_size << " bytes) to N64" << std::endl;

    std::ifstream file(asset_path, std::ios::binary);

    BeginMessage begin_message (file_name, file_size);
    DataMessage data_message;
    
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
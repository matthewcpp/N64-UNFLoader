#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

std::string rom_path = "D:/development/repos/framework64/build_n64/bin/data_link.z64";

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

        std::string message(buf.data(), size);
        std::cout << "N64: " << message << std::endl;
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

struct DataMessage {
    char payload[2048];
};

void send_file(std::string const & path_str) {
    std::filesystem::path asset_path = path_str;

        auto file_name = asset_path.filename().string();
        auto file_size = static_cast<size_t>(std::filesystem::file_size(asset_path));

        std::ifstream file(asset_path, std::ios::binary);

        
        BeginMessage begin_message (file_name, file_size);

        //std::cout << "Sending " << file_size << " bytes to N64" << std::endl;
        process_incomming_messages();
        device_senddata(DATATYPE_TEXT, reinterpret_cast<char*>(&begin_message), sizeof(BeginMessage));
        //device_senddata(DATATYPE_TEXT, "Hello World", strlen("Hello World"));
}

int main(int argc, char** argv) {
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

    //process_incomming_messages();
    std::string asset_path_str;

    for (;;) {
        std::cout << "Enter asset path:" << std::endl;
        std::getline(std::cin, asset_path_str);

        if (asset_path_str == ".exit")
            break;

        std::string file_path = "D:/development/repos/framework64/build_n64/bin/data_link/assets/controller_cube.mesh";
        send_file(file_path);

/*
        if (!std::filesystem::exists(asset_path_str)) {
            std::cout << "invalid path specified." << std::endl;
            continue;
        }
*/
    }



    return 0;
}
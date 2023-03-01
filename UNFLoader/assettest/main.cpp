#include "device.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

std::string rom_path = "D:/development/repos/framework64/build_n64/bin/data_link.z64";
bool waiting_for_data_awk = false;
uint32_t bytes_awk = 0;
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
            waiting_for_data_awk = false;
            uint32_t big_endian;
            std::memcpy(&big_endian, buf.data(), sizeof(uint32_t));

            uint32_t little_endian = _byteswap_ulong(big_endian);
            bytes_awk += little_endian;
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

    bytes_awk = 0;
    size_t data_sent = 0;
    while (data_sent < file_size) {
        size_t data_remaining = file_size - data_sent;
        size_t amount_to_send = data_remaining > DATA_MESSAGE_PAYLOAD_SIZE ? DATA_MESSAGE_PAYLOAD_SIZE : data_remaining;
        data_sent += amount_to_send;

        data_message.payload_size = _byteswap_ulong(amount_to_send);
        file.read(&data_message.payload[0], amount_to_send);

        waiting_for_data_awk = true;
        device_senddata(2, reinterpret_cast<char*>(&data_message), sizeof(DataMessage));

#if 0
        while (waiting_for_data_awk) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            process_incomming_messages();
        }
#endif
    }

    std::cout << "Transfer Complete" << std::endl;
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

        if (!std::filesystem::exists(asset_path_str)) {
            std::cout << "invalid path specified." << std::endl;
            continue;
        }

        // std::string file_path = "D:/development/repos/framework64/build_n64/bin/data_link/assets/controller_cube.mesh";

        send_file(asset_path_str);
    }



    return 0;
}
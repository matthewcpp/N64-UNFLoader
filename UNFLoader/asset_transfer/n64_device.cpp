#include "n64_device.hpp"

#include "device.h"

#include "device_messages.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

namespace framework64::asset_transfer {

void on_device_error(const char* error);
void on_sendrom(float percent);
void on_device_message(const char* message);
void process_incomming_messages();

const char* N64Device::identifier() const {
    return "N64";
}

bool N64Device::initialize(std::string const & rom_path) {
    _rom_path = rom_path;

    device_sendrom_params_t sendrom_params;
    device_sendrom_params_init(&sendrom_params);

    device_set_fatal_error_callback(on_device_error);
    device_set_message_callback(on_device_message);
    device_set_sendrom_progress_callback(on_sendrom);

    device_find(CART_ANY);
    device_open();

    device_sendrom(_rom_path.c_str(), &sendrom_params);
    std::cout << "ROM Uploaded to device" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return true;
}

bool N64Device::transferStaticMesh(std::string const & path) {
    std::filesystem::path asset_path = path;

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

    return true;
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

void on_device_error(const char* error) {
    std::cout << "Fatal Error: " << error;
}

void on_sendrom(float percent) {
    if (percent == 1.0f) {
        std::cout << "Rom Uploaded." << std::endl;
    }
}

void on_device_message(const char* message) {
    std::cout << "Device message:" << message;
}

}
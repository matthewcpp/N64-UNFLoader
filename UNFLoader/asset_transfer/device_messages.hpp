#pragma once

#include <cstdint>
#include <string>

//TODO: add additional swapping intrinsics : https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c

namespace framework64::asset_transfer {

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

}
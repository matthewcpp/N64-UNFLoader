#include "processor.hpp"

#include <iostream>

namespace framework64::asset_transfer {

uuids::uuid_random_generator init_uuid();

Processor::Processor(Settings const & settings, DeviceInterface & di) : 
    converter(settings), device_interface(di), uuid_generator(init_uuid()) {

    temp_dir = std::filesystem::temp_directory_path();
}

bool Processor::processStaticMesh(std::string const & file) {
    auto uuid = uuids::to_string(uuid_generator());

    std::filesystem::path file_path = file;
    std::filesystem::path scratch_dir = temp_dir / uuid;

    std::cout << "Processing file: " << file << std::endl;
    std::cout << "Scratch dir: " << scratch_dir.string() << std::endl;

    std::filesystem::create_directory(scratch_dir);
    auto converted_file_path = converter.convertStaticMesh(file_path, scratch_dir);

    auto file_was_converted = std::filesystem::exists(converted_file_path);
    if (file_was_converted) {
        device_interface.transferStaticMesh(converted_file_path);
    }

    std::filesystem::remove_all(scratch_dir);

    return file_was_converted;
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
    
}
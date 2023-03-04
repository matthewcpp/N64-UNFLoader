#pragma once

#include "device_interface.hpp"

#include <chrono>
#include <thread>

namespace framework64::asset_transfer {

class NullDevice : DeviceInterface {
public:
    size_t transfer_time_miliseconds = 1000;

    virtual bool transferStaticMesh(std::string const & path) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(transfer_time_miliseconds));
    }

    const char* identifier() const override {
        return "null";
    }
};

}
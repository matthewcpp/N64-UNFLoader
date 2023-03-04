#pragma once

#include "processor.hpp"

#include <memory>

namespace framework64::asset_transfer {

class WebService {
public:
    WebService(Processor & processor);
    ~WebService();

    void run();
private:
    class Handler;
    std::unique_ptr<Handler> handler;
};

} 
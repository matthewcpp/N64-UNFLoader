#pragma once

#include "processor.hpp"


namespace framework64::asset_transfer {

class WebService {
public:
    WebService(Processor & p) : processor(p) {}

    void run();
private:

    Processor & processor;
};

} 
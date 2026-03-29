#pragma once

#include "common.hpp"

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class BubblerSystem {

public:
    void step(float elapsed_ms);

private:
    // Distance player must be from bubbler to charge
    const float DISTANCE_THRESHOLD = 100.f; 

    // Time to charge a bubbler (ms)
    const float CHARGE_TIME = 10000.f; 
};
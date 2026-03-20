#pragma once

#include <iostream>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Growth over time
class GrowthSystem {
  public:
    void step(float elapsed_ms);

    GrowthSystem() {}
};

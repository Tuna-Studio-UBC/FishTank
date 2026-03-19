#pragma once

#include <iostream>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Handle expiring effects
class ExpirationSystem {
  public:
    void step(float elapsed_ms);

    ExpirationSystem() {}
};

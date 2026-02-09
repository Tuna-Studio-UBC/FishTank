#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class AISystem {
  public:
    void step(float elapsed_ms);
    void handle_detection();
};
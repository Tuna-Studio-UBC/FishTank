#pragma once

#include <memory>

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class TransitionSystem {
  public:
    void step(float elapsed_ms);

    static std::unique_ptr<State> create_state(STATE_TYPE type);
};

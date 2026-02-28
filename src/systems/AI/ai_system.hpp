#pragma once
#include <cmath>
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include "swarm_system.hpp"

class AISystem {
  private:
    void act(Entity e, float elapsed_ms, vec2 player_pos);

  public:
    void step(float elapsed_ms);
    void handle_detection();
};

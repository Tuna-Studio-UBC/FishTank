#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  public:
    void step(float elapsed_ms);

    PhysicsSystem() {}
};
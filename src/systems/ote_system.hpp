#pragma once

#include <iostream>

#include "../common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Handle applying effects over time (e.g. health regen, DoT)
class OverTimeEffectSystem {
  public:
    void step(float elapsed_ms);

    OverTimeEffectSystem() {}

  private:
    void handle_heal(Entity e, Entity target_entity);
    void handle_damage(Entity e, Entity target_entity);
};
#pragma once

#include "../common.hpp"
#include "../world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Updates player and enemy health
class HealDamageSystem {
  public:
    void step(float elapsed_ms);

    HealDamageSystem() {}

    static void add_damage(Entity entity, int amount);
    static void add_heal(Entity entity, int amount);

  private:
    void handle_damaging();
    void handle_healing();
};
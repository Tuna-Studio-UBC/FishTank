#pragma once

#include "common.hpp"
#include "systems/world/world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Updates player and enemy health
class HealDamageSystem {
  public:
    void step(float elapsed_ms);

    HealDamageSystem() {}

    static void add_damage(Entity entity, int amount, vec3 tint = {1.f, 0.25f, 0.25f});
    static void add_heal(Entity entity, int amount, vec3 tint = {0.25f, 1.f, 0.25f});

  private:
    float tint_duration_ms = 200.f;

    void handle_damaging();
    void handle_healing();
};

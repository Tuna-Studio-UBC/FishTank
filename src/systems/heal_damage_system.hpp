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
    float tint_duration_ms = 250.f;
    vec3 damage_tint = vec3(1.f, 0.25f, 0.25f);
    vec3 heal_tint = vec3(0.25f, 1.f, 0.25f);

    void handle_damaging();
    void handle_healing();
};

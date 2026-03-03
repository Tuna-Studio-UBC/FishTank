#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Updates the player's state based on movement
class PlayerInputSystem {
  public:
    void step(float elapsed_ms);

    PlayerInputSystem() {}

  private:
    const float DEFAULT_BARREL_LEN = 30.f;

    int get_player_ammo_cost();

    void try_fire_projectile(Motion &player_motion, FireControl &fc, const float &aim_angle);
};

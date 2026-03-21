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
    float game_time_ms = 0.0f;

    int get_player_ammo_cost();

    void try_fire_left_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle);
    void try_fire_right_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle);
    void process_burst_fire(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle);
};

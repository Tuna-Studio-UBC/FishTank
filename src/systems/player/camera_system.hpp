#pragma once
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class CameraSystem {
  public:
    void step();

    CameraSystem() {}

    void init(Entity current_map);

    void set_current_map(Entity current_map) { this->current_map = current_map; }

    void set_camera_bounds();

  private:
    vec2 get_camera_dest();

    vec2 min_pos = vec2(0.f, 0.f);
    vec2 max_pos = vec2(0.f, 0.f);

    Entity current_map;
    const float INTERP_COEFFICIENT = 0.1f;
    const float MOUSE_SENSITIVITY_FACTOR = 0.2f;
    const float MAX_OFFSET = 50.f;
    const int TILE_OOB_BUFFER = 1; // Number of tiles the player can see out of bounds
};

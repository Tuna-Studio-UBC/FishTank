#pragma once
#include "../common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class CameraSystem {
  public:
    void step();

    CameraSystem() {}

	void init();

private:

	vec2 get_camera_dest();

	const float INTERP_COEFFICIENT = 0.03f;
	const float MOUSE_SENSITIVITY_FACTOR = 0.2f;
	const float MAX_OFFSET = 50.f;
	const int TILE_OOB_BUFFER = 1; // Number of tiles the player can see out of bounds

};

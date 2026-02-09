#include "camera_system.hpp"

void CameraSystem::step() {
    auto &camera = registry.camera.entities[0];
    auto &player = registry.players.entities[0];
    vec2 &camera_pos = registry.motions.get(camera).position;
    vec2 &player_pos = registry.motions.get(player).position;

    camera_pos = camera_pos * (1 - interp_coefficient) + player_pos * interp_coefficient;
}
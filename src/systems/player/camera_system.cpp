#include "camera_system.hpp"
#include "systems/world/world_init.hpp"

void CameraSystem::step() {
    auto &camera = registry.camera.entities[0];
    vec2 &camera_pos = registry.motions.get(camera).position;
    auto camera_dest = get_camera_dest();

    camera_pos = camera_pos * (1 - INTERP_COEFFICIENT) + camera_dest * INTERP_COEFFICIENT;
}

vec2 CameraSystem::get_camera_dest() {
    auto &player = registry.players.entities[0];
    vec2 &player_pos = registry.motions.get(player).position;
    vec2 &mouse_pos = registry.mouseInput.components[0].mouse_pos;

    if (mouse_pos == vec2(0))
        mouse_pos = player_pos;

    vec2 offset = (mouse_pos - vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2)) * MOUSE_SENSITIVITY_FACTOR;
    offset = glm::clamp(offset, vec2(-MAX_OFFSET), vec2(MAX_OFFSET));

    return glm::clamp(offset + player_pos, min_pos, max_pos);
}

void CameraSystem::set_camera_bounds() {
    assert(registry.tilemaps.has(current_map));
    TileMap &tilemap = registry.tilemaps.get(current_map);
    min_pos = vec2(TILE_SIZE_PX * -TILE_OOB_BUFFER + (WINDOW_WIDTH_PX / 2),
                   TILE_SIZE_PX * -TILE_OOB_BUFFER + (WINDOW_HEIGHT_PX / 2));
    max_pos = vec2(TILE_SIZE_PX * (TILE_OOB_BUFFER + tilemap.width) - (WINDOW_WIDTH_PX / 2),
                   TILE_SIZE_PX * (TILE_OOB_BUFFER + tilemap.height) - (WINDOW_HEIGHT_PX / 2));
    if (min_pos.x > max_pos.x) {
        float average_x = (min_pos.x + max_pos.x) / 2.f;
        min_pos.x = average_x;
        max_pos.x = average_x;
    }
    if (min_pos.y > max_pos.y) {
        float average_y = (min_pos.y + max_pos.y) / 2.f;
        min_pos.x = average_y;
        max_pos.x = average_y;
    }
}

void CameraSystem::init(Entity current_map) {
    this->current_map = current_map;
    set_camera_bounds();
    if (registry.camera.entities.empty()) {
        if (registry.players.entities.size() > 0)
            createCamera(get_camera_dest());
        else
            createCamera(get_camera_dest());
    }
}

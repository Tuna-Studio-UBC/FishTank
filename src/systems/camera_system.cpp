#include "camera_system.hpp"
#include "world_init.hpp"

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

    vec2 offset = (mouse_pos - vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2)) * MOUSE_SENSITIVITY_FACTOR;
    offset = glm::clamp(offset, vec2(-MAX_OFFSET), vec2(MAX_OFFSET));

    int tilemap_width = registry.tilemaps.components[0].width;
    int tilemap_height = registry.tilemaps.components[0].height;

    vec2 min_pos = vec2(TILE_SIZE_PX * -TILE_OOB_BUFFER + (WINDOW_WIDTH_PX / 2),
                        TILE_SIZE_PX * -TILE_OOB_BUFFER + (WINDOW_HEIGHT_PX / 2));
    vec2 max_pos = vec2(TILE_SIZE_PX * (TILE_OOB_BUFFER + tilemap_width) - (WINDOW_WIDTH_PX / 2),
                        TILE_SIZE_PX * (TILE_OOB_BUFFER + tilemap_height) - (WINDOW_HEIGHT_PX / 2));
    // std::cout << "Offset: (" << (offset + player_pos).x << ", " << (offset + player_pos).y << ") | ";
    // std::cout << "Min Pos: (" << min_pos.x <<", " << min_pos.y << ") | ";
    // std::cout << "Max Pos: (" << max_pos.x <<", " << max_pos.y << ")" << std::endl;
    return glm::clamp(offset + player_pos, min_pos, max_pos);
}

void CameraSystem::init() {
    // Only one camera is allowed
    if (registry.camera.entities.empty())
        createCamera(get_camera_dest());
}

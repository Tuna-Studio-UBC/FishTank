#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "utilities/parse_tiled.hpp"

// TODO: create entity functions
// Initiate tile map
void init_tiles();

// Create player entity
Entity createPlayer(RenderSystem *renderer, vec2 position);

Entity createEnemy(RenderSystem *renderer, vec2 position, vec2 patrol_a, vec2 patrol_b, float speed_px_s);
Entity createBell(RenderSystem* renderer, vec2 position, float interact_radius_px);

Entity createProjectile(RenderSystem* renderer, vec2 position, float angle);

// Create enemy entity
Entity createEnemy(RenderSystem* renderer, vec2 position, float radius);

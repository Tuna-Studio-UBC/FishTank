#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "utilities/parse_tiled.hpp"
#include <configs/enemy_config.hpp>
#include <configs/attack_pattern.hpp>

// TODO: create entity functions
// Initiate tile map
void init_tiles();

// Create player entity
Entity createPlayer(RenderSystem *renderer, vec2 position);

Entity createBell(RenderSystem* renderer, vec2 position, float interact_radius_px);

Entity createProjectile(RenderSystem* renderer, vec2 position, float angle, ATTACK_PATTERN attack_pattern);

// Create enemy entity
Entity createEnemy(RenderSystem* renderer, vec2 position, ENEMY_TYPE type);
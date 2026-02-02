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

// Create enemy entity
Entity createEnemy(RenderSystem* renderer, vec2 position, float radius);

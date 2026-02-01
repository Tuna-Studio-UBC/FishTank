#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// TODO: create entity functions
Entity createPlayer(RenderSystem* renderer, vec2 position);
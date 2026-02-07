#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// Updates the player's state based on movement
class PlayerInputSystem
{
public:
    void step(float elapsed_ms);

    PlayerInputSystem()
    {
    }
};
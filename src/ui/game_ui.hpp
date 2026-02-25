#pragma once

#include <iostream>

#include "imgui.h"

#include "../common.hpp"
#include "systems/world_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "utils.hpp"

struct GameUI {
    void draw_healthbar();
    void draw_ammo();
    void draw_respawn(WorldSystem *world_system);
    inline void draw_demo() { ImGui::ShowDemoWindow(); }
};

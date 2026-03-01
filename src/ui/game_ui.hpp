#pragma once

#include <iostream>

#include "imgui.h"

#include "../common.hpp"
#include "systems/world_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "utils.hpp"

#include "ui/game/tutorial/tutorial_ui.hpp"

struct GameUI {
    void draw_fps_counter(WorldSystem *world_system);
    void draw_healthbar();
    void draw_ammo();
    void draw_respawn(WorldSystem *world_system);
    void draw_minimap();
    inline void draw_demo() { ImGui::ShowDemoWindow(); }
};

static GameUI game_ui;

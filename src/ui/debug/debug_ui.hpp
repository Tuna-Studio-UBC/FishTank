#pragma once

#include <iostream>

#include "imgui.h"

#include "common.hpp"
#include "systems/world/world_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "ui/utils.hpp"

struct DebugUI {
    void draw_fps_counter(WorldSystem *world_system);
    void draw_respawn(WorldSystem *world_system);
    inline void draw_demo() { ImGui::ShowDemoWindow(); }
};

static DebugUI debug_ui;

#pragma once

#include <iostream>

#include "imgui.h"

#include "utils.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "../common.hpp"
#include "world_system.hpp"

struct GameUI
{
  void draw_healthbar();
  void draw_respawn(WorldSystem *world_system);
  inline void draw_demo() { ImGui::ShowDemoWindow(); }
};
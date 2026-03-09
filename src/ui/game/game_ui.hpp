#pragma once

#include <iostream>

#include "imgui.h"

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "ui/utils.hpp"

struct GameUI {
    void draw_healthbar();
    void draw_enemy_healthbars();
    void draw_ammo();
    void draw_minimap();
    void draw_pause_menu();
    void draw_cursor();
};

static GameUI game_ui;

#pragma once

#include <iostream>

#include "imgui.h"

#include "common.hpp"
#include "systems/player/upgrade_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "ui/utils.hpp"

struct GameUI {
    void set_upgrade_system(UpgradeSystem *upgrade_system);
    void draw_healthbar();
    void draw_enemy_healthbars();
    void draw_boss_healthbars();
    void draw_ammo();
    void draw_minimap();
    void draw_wallet();
    void draw_score();
    void draw_pause_menu();
    void draw_upgrade_menu();
    void draw_cursor();

  private:
    UpgradeSystem *upgrade_system = nullptr;
};

static GameUI game_ui;

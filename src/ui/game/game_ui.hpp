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
    void draw_ammo();
    void draw_minimap();
    void draw_pause_menu();
    void draw_upgrade_menu();
};

static GameUI game_ui;

void try_upgrade_player(UPGRADE_TYPE type);

void upgrade_max_ammo(Entity &player, UpgradeInstance &upgrade_instance);
void upgrade_max_hp(Entity &player, UpgradeInstance &upgrade_instance);
void upgrade_attack(Entity &player, UpgradeInstance &upgrade_instance);

void close_upgrade_menu();
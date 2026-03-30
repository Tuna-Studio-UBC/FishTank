#pragma once

#include <iostream>

#include "imgui.h"

#include "common.hpp"
#include "systems/audio/audio_system.hpp"
#include "systems/player/upgrade_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "ui/utils.hpp"

class LevelSystem;

struct GameUI {
    void set_upgrade_system(UpgradeSystem *upgrade_system);
    void set_audio_system(AudioSystem *audio_system);
    void set_level_system(LevelSystem *level_system);
    void draw_healthbar();
    void draw_enemy_healthbars();
    void draw_boss_healthbars();
    void draw_ammo();
    void draw_ammo_types();
    void draw_minimap();
    void draw_wallet();
    void draw_score();
    void draw_pause_menu();
    void draw_upgrade_menu();
    void draw_cursor();

  private:
    UpgradeSystem *upgrade_system = nullptr;
    AudioSystem *audio_system = nullptr;
    LevelSystem *level_system = nullptr;
};

// prevent linker error
extern GameUI game_ui;

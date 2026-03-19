#pragma once

#include <iostream>

#include "imgui.h"

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "ui/utils.hpp"

struct TutorialUI {
    void draw_tutorial_step();
    void draw_tutorial_movement(TutorialStep tutorial);
    void draw_tutorial_hazards(TutorialStep tutorial);
    void draw_tutorial_healing(TutorialStep tutorial);
    void draw_tutorial_ammo(TutorialStep tutorial);
    void draw_tutorial_combat(TutorialStep tutorial);
    void draw_tutorial_upgrade_system(TutorialStep tutorial);
};

static TutorialUI tutorial_ui;

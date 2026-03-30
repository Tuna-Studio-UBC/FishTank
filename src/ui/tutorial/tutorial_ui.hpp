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
    void draw_tutorial_skip_button();

    void draw_tutorial_movement(TutorialStep tutorial);
    void draw_tutorial_interactables(TutorialStep tutorial);
    void draw_tutorial_combat(TutorialStep tutorial);
};

static TutorialUI tutorial_ui;

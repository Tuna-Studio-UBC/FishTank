#define GL3W_IMPLEMENTATION
#include <gl3w.h>

#include <spdlog/spdlog.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "systems/ai_system.hpp"
#include "systems/animation_system.hpp"
#include "systems/camera_system.hpp"
#include "systems/collision_system.hpp"
#include "systems/death_system.hpp"
#include "systems/heal_damage_system.hpp"
#include "systems/ote_system.hpp"
#include "systems/particle_system.hpp"
#include "systems/physics_system.hpp"
#include "systems/pick_up_system.hpp"
#include "systems/player_input_system.hpp"
#include "systems/render_system.hpp"
#include "systems/world_system.hpp"

#include "ui/game_ui.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main() {
    // global systems
    AISystem ai_system;
    CollisionSystem collision_system;
    OverTimeEffectSystem ote_system;
    HealDamageSystem heal_damage_system;
    DeathSystem death_system;
    PickUpSystem pick_up_system;
    WorldSystem world_system;
    RenderSystem renderer_system;
    PhysicsSystem physics_system;
    PlayerInputSystem player_input_system;
    CameraSystem camera_system;
    AnimationSystem animation_system;

    GameUI game_ui;

    // Configure spdlog
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l]%$ %v");
    spdlog::set_level(spdlog::level::debug);

    // initialize window
    GLFWwindow *window = world_system.create_window();
    if (!window) {
        // Time to read the error message
        spdlog::error("Failed to create window.  Press any key to exit");
        getchar();
        return EXIT_FAILURE;
    }

    // Start and load audio
    if (!world_system.start_and_load_sounds()) {
        spdlog::error("Failed to start or load sounds.");
    }

    // initialize the main systems
    renderer_system.init(window);
    world_system.init(&renderer_system, &camera_system);

    // variable timestep loop
    auto t = Clock::now();
    while (!world_system.is_over()) {
        // Prepare new GUI frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // processes system messages, if this wasn't present the window would become unresponsive
        glfwPollEvents();

        // calculate elapsed times in milliseconds from the previous iteration
        auto now = Clock::now();
        float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
        t = now;

        // CK: be mindful of the order of your systems and rearrange this list only if necessary

        if (world_system.step(elapsed_ms)) {
            ote_system.step(elapsed_ms);
            heal_damage_system.step(elapsed_ms);
            death_system.step(elapsed_ms);
            pick_up_system.step(elapsed_ms);
            ai_system.handle_detection();
            ai_system.step(elapsed_ms);
            physics_system.step(elapsed_ms);
            player_input_system.step(elapsed_ms);
            world_system.handle_expiration(elapsed_ms);
            particle_system.step(elapsed_ms);
            animation_system.step(elapsed_ms);
            collision_system.step(elapsed_ms);
            camera_system.step();
        }

        // Draw GUIs
        game_ui.draw_healthbar();
        game_ui.draw_ammo();

// Debugging UI
#ifdef ENABLE_DEBUG_UI
        game_ui.draw_demo();
        game_ui.draw_respawn(&world_system);
#endif

        // Render
        renderer_system.draw();
        renderer_system.renderGUI();

        // flicker-free display with a double buffer
        glfwSwapBuffers(window);
        gl_has_errors();
    }

    renderer_system.shutdown();

    return EXIT_SUCCESS;
}

#define GL3W_IMPLEMENTATION
#define TracyLine TracyConcat(__LINE__, U) // Fix for MSVC
#include <gl3w.h>

#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "systems/ai/ai_system.hpp"
#include "systems/fsm/fsm_system.hpp"
#include "systems/fsm/transition_system.hpp"
#include "systems/health/death_system.hpp"
#include "systems/health/heal_damage_system.hpp"
#include "systems/level/level_system.hpp"
#include "systems/physics/collision_system.hpp"
#include "systems/physics/physics_system.hpp"
#include "systems/player/camera_system.hpp"
#include "systems/player/pick_up_system.hpp"
#include "systems/player/player_input_system.hpp"
#include "systems/rendering/animation_system.hpp"
#include "systems/rendering/particle_system.hpp"
#include "systems/rendering/render_system.hpp"
#include "systems/timed/expiration_system.hpp"
#include "systems/timed/growth_system.hpp"
#include "systems/timed/ote_system.hpp"
#include "systems/world/tilemap_system.hpp"
#include "systems/world/world_system.hpp"

#include "ui/debug/debug_ui.hpp"
#include "ui/game/game_ui.hpp"
#include "ui/tutorial/tutorial_ui.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main() {
    // global systems
    AISystem ai_system;
    PathfindingSystem path_system;
    CollisionSystem collision_system;
    ExpirationSystem expiration_system;
    FSMSystem fsm_system;
    GrowthSystem growth_system;
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
    TileMapSystem tilemap_system;
    TransitionSystem transition_system;
    UpgradeSystem upgrade_system;
    LevelSystem level_system(&tilemap_system, &renderer_system, &camera_system, &world_system);

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

    // initialize game state (done outside of systems)
    registry.progresses.emplace(Entity("Progress"));

    // initialize the main systems
    renderer_system.init(window);
    world_system.init(&renderer_system, &camera_system, &tilemap_system, &upgrade_system);
    level_system.load_tutorial();
    game_ui.set_upgrade_system(&upgrade_system);
    game_ui.set_audio_system(world_system.get_audio_system());
    camera_system.init(tilemap_system.get_current_map());
    path_system.init(&tilemap_system);
    ai_system.init(&path_system);

    // variable timestep loop
    auto t = Clock::now();
    while (!world_system.is_over()) {
        // Prepare new GUI frame
        {
            ZoneScopedN("EXT: IMGUI Prep Frame");
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        // processes system messages, if this wasn't present the window would become unresponsive
        {
            ZoneScopedN("EXT: GLFW Poll Events");
            glfwPollEvents();
        }

        // calculate elapsed times in milliseconds from the previous iteration
        auto now = Clock::now();
        float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
        t = now;

        // CK: be mindful of the order of your systems and rearrange this list only if necessary

        if (world_system.step(elapsed_ms)) {
            {
                ZoneScopedN("SYS: Timed");
                growth_system.step(elapsed_ms);
                ote_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: HealDamage");
                heal_damage_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Death");
                death_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: PickUp");
                pick_up_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: State Machines");
                fsm_system.step(elapsed_ms);
                transition_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: AI Detection");
                ai_system.handle_detection();
            }
            {
                ZoneScopedN("SYS: AI Step");
                ai_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Physics");
                physics_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: PlayerInput");
                player_input_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Expiration");
                expiration_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Particles");
                particle_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Animation");
                animation_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Collision");
                collision_system.step(elapsed_ms);
            }
            {
                ZoneScopedN("SYS: Camera");
                camera_system.step();
            }
        }
        // Handle level loading and resetting
        {
            ZoneScopedN("SYS: Level");
            level_system.step(elapsed_ms);
        }

        // Draw GUIs
        {
            ZoneScopedN("RENDER: Health");
            game_ui.draw_enemy_healthbars();
            game_ui.draw_boss_healthbars();
            game_ui.draw_healthbar();
        }
        {
            ZoneScopedN("RENDER: Minimap");
            game_ui.draw_minimap();
        }
        {
            ZoneScopedN("RENDER: Wallet");
            game_ui.draw_wallet();
        }
        {
            ZoneScopedN("RENDER: Wallet");
            game_ui.draw_score();
        }
        {
            ZoneScopedN("RENDER: Ammo");
            game_ui.draw_ammo();
        }
        {
            ZoneScopedN("RENDER: Upgrade");
            game_ui.draw_upgrade_menu();
        }
        {
            ZoneScopedN("RENDER: Pause");
            game_ui.draw_pause_menu();
        }

        {
            ZoneScopedN("RENDER: Tutorial");
            tutorial_ui.draw_tutorial_step();
        }

        // Debugging UI
        {
            ZoneScopedN("RENDER: FPS");
            debug_ui.draw_fps_counter(&world_system);
        }

#ifdef ENABLE_DEBUG_UI
        debug_ui.draw_demo();
        debug_ui.draw_respawn(&world_system);
#endif

        {
            ZoneScopedN("RENDER: Cursor");
            game_ui.draw_cursor();
        }

        // Render
        {
            ZoneScopedN("RENDER: Main Draw Call");
            renderer_system.draw();
        }
        {
            ZoneScopedN("RENDER: Main GUI Call");
            renderer_system.renderGUI();
        }

        // flicker-free display with a double buffer
        {
            ZoneScopedN("RENDER: SwapBuffer");
            glfwSwapBuffers(window);
        }
        gl_has_errors();
    }

    renderer_system.shutdown();

    return EXIT_SUCCESS;
}

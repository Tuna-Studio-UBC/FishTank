// Header
#include "systems/world/world_system.hpp"
#include "systems/world/world_init.hpp"
#include <tracy/Tracy.hpp>
#define TracyLine TracyConcat(__LINE__, U) // Fix for MSVC

// stlib
#include "configs/wave_config.cpp"
#include "io/persist_file.hpp"
#include "systems/physics/physics_system.hpp"
#include "systems/rendering/particle_system.hpp"
#include <cassert>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <unordered_set>

// DEBUG: DELETE ME
#include "ui/game/game_ui.hpp"

// create the world
WorldSystem::WorldSystem() {
    // seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
    audio_system.shutdown();
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();

    // Close the window
    glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char *desc) { spdlog::error("GLFW Error {}: {}", error, desc); }
} // namespace

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window() {
    ///////////////////////////////////////
    // Initialize GLFW
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        spdlog::error("ERROR: Failed to initialize GLFW in world_system.cpp");
        return nullptr;
    }

    //-------------------------------------------------------------------------
    // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
    // enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
    // GLFW / OGL Initialization
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
    // TODO: Change this to true then handle different scalings
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE); // GLFW 3.3+

    // Create the main window (for rendering, keyboard, and mouse input)
    window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, GAME_NAME, nullptr, nullptr);
    if (window == nullptr) {
        spdlog::error("ERROR: Failed to glfwCreateWindow in world_system.cpp");
        return nullptr;
    }

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(window, this);
    auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3) {
        ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
    };
    auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1) {
        ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
    };
    auto mouse_button_pressed_redirect = [](GLFWwindow *wnd, int _button, int _action, int _mods) {
        ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods);
    };

    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

    // Center window
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    const int x = (mode->width - WINDOW_WIDTH_PX) / 2;
    const int y = (mode->height - WINDOW_HEIGHT_PX) / 2;
    glfwSetWindowPos(window, x, y);

    return window;
}

bool WorldSystem::start_and_load_sounds() {

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    if (!audio_system.load()) {
        fprintf(stderr, "Failed to load SFX\n");
    }

    audio_system.play_bgm("bgm.wav", 45);

    return true;
}

void WorldSystem::clear_level_objects() {
    for (auto entity_list : entities_to_clear)
        for (auto entity : *entity_list)
            registry.remove_all_components_of(entity);
}

void WorldSystem::try_progress_tutorial() {
    auto &progress = registry.progresses.components[0];

    if (!progress.is_tutorial)
        return;

    if (progress.is_tutorial_step_called && !(progress.tutorial_step_id > progress.tutorial_steps.size())) {
        // Progress to next step
        TutorialStep &step = progress.tutorial_steps[progress.tutorial_step_id - 1];

        if (step.current_step >= step.total_steps) {
            progress.is_paused = false;
            progress.is_tutorial_step_called = false;
        } else {
            step.current_step++;
            spdlog::debug("[TUTORIAL] Progressed to step {} of tutorial #{}", step.current_step, (int)step.id);
        }
    } else if (progress.is_game_over && progress.has_beat_level) {
        // Load into main game
        Health &health = registry.healths.get(registry.players.entities[0]);
        health.hp = health.max_hp;
        FireControl &fc = registry.fireControls.get(registry.players.entities[0]);
        fc.ammo = fc.max_ammo;

        // load_next_level();
    }
}

void WorldSystem::init(RenderSystem *renderer_arg, CameraSystem *camera_arg, TileMapSystem *tilemap_arg,
                       UpgradeSystem *upgrade_system) {
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;
    this->tilemap_system = tilemap_arg;
    this->upgrade_system = upgrade_system;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
    ZoneScopedN("SYS: World");

    // Updating window title
    std::stringstream title_ss;
    title_ss << GAME_NAME;
    glfwSetWindowTitle(window, title_ss.str().c_str());

    // Updating FPS
    fps_index = (fps_index + 1) % fps_avg.size();
    fps_avg[fps_index] = 1000.0f / elapsed_ms_since_last_update;

    Progress &progress = registry.progresses.components[0];

    // Check if game is over
    if (progress.is_game_over == true) {
        ScreenState &screen = registry.screenStates.components[0];
        if (screen.darken_screen_factor < 0.9f) {
            screen.darken_screen_factor += elapsed_ms_since_last_update / 1000.f;
        }
        return false;
    }

    // Check if game is paused
    if (progress.is_paused)
        return false;

    for (Entity b : registry.bells.entities) {
        Bell &bell = registry.bells.get(b);
        if (bell.cooldown_timer > 0.f) {
            bell.cooldown_timer -= elapsed_ms_since_last_update;
            if (bell.cooldown_timer < 0.f)
                bell.cooldown_timer = 0.f;
        }
    }

    vec2 listener_pos = vec2(0.f, 0.f);
    if (registry.players.entities.size() > 0 && registry.motions.has(registry.players.entities[0])) {
        listener_pos = registry.motions.get(registry.players.entities[0]).position;
    }

    // Create bullet entities requested by enemies in AISystem
    std::unordered_set<unsigned int> shot_played;
    for (BulletRequest req : registry.bullet_requests.components) {
        if (registry.enemies.has(req.owner)) {

            unsigned int oid = (unsigned int)req.owner;
            if (shot_played.find(oid) == shot_played.end()) {
                shot_played.insert(oid);
                vec2 enemy_pos = req.position;
                if (registry.motions.has(req.owner)) {
                    enemy_pos = registry.motions.get(req.owner).position;
                }

                audio_system.play_enemy_shot(enemy_pos, listener_pos);
            }
        }

        createProjectile(req.position, req.angle, req.attack_pattern, req.owner);
    }
    registry.bullet_requests.clear();

    for (auto &sr : registry.sound_requests.components) {
        if (sr.type == SOUND_EVENT::PLAYER_SHOT) {
            audio_system.play_player_shot_center(128);
        }
    }
    registry.sound_requests.clear();

    // Remove debug info from the last step
    while (registry.debugComponents.entities.size() > 0)
        registry.remove_all_components_of(registry.debugComponents.entities.back());

    audio_system.update_hazard_warning(listener_pos, elapsed_ms_since_last_update);

    return true;
}

void WorldSystem::respawn_player() {
    Entity &player = registry.players.entities[0];
    registry.remove_all_components_of(player);
    registry.players.remove(player);
    createPlayer(renderer, {WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX / 2.0f});
}

// Should the game be over ?
bool WorldSystem::is_over() const { return bool(glfwWindowShouldClose(window)); }

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

    // We should have 1 and only ever 1 player input component
    assert(registry.keyboardInputs.components.size() == 1);
    Entity playerEntity = registry.players.entities[0];
    KeyboardInputState &ps = registry.keyboardInputs.get(playerEntity);

    bool key_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

    // We handle the inputs separately since we need to deal with game logic
    // E.g. how to handle multiple conflicting inputs
    if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
        ps.current_actions[KeyboardActions::MoveForward] = key_pressed;
    }
    if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
        ps.current_actions[KeyboardActions::TurnLeft] = key_pressed;
        ps.most_recent_turn = KeyboardActions::TurnLeft;
    }
    if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
        ps.current_actions[KeyboardActions::TurnRight] = key_pressed;
        ps.most_recent_turn = KeyboardActions::TurnRight;
    }
    if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        ps.current_actions[KeyboardActions::QuickTurn] = key_pressed;
    }

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_E) {
            if (registry.players.entities.size() == 0) {
                return;
            }

            Entity player = registry.players.entities[0];
            if (!registry.motions.has(player)) {
                return;
            }

            vec2 player_pos = registry.motions.get(player).position;
            float best_dist2 = 1e30f;
            Entity best_bell;

            for (Entity b : registry.bells.entities) {
                if (!registry.motions.has(b))
                    continue;
                vec2 bell_pos = registry.motions.get(b).position;
                vec2 d = bell_pos - player_pos;
                float dist2 = dot(d, d);
                if (dist2 < best_dist2) {
                    best_dist2 = dist2;
                    best_bell = b;
                }
            }

            if (best_dist2 > 9e29f) {
                return;
            }

            Bell &bell = registry.bells.get(best_bell);
            float radius = bell.interact_radius_px;
            float radius2 = radius * radius;

            if (best_dist2 > radius2) {
                float dist = sqrtf(best_dist2);
                return;
            }

            if (bell.cooldown_timer > 0.f) {
                spdlog::debug("[Bell] {} on cooldown: {:.2f}ms", best_bell.name_id(), bell.cooldown_timer);
                return;
            }

            vec2 bell_pos = registry.motions.get(best_bell).position;

            for (Entity enemy : registry.enemies.entities) {
                if (!registry.motions.has(enemy)) {
                    continue;
                }
                Motion &enemy_motion = registry.motions.get(enemy);
                if (entity_in_radius(bell_pos - enemy_motion.position, enemy_motion.scale, bell.distraction_radius)) {
                    Sense &sense = registry.senses.get(enemy);
                    sense.is_distracted = true;
                    Distracted &distracted = registry.distracteds.get(enemy);
                    distracted.bell = best_bell;
                    distracted.remaining_ms = bell.distraction_ms;
                }
            }

            bell.cooldown_timer = bell.cooldown_ms;

            // spdlog::debug("[Bell] {} triggered at ({:.2f}, {:.2f})", best_bell.name_id(), bell_pos.x, bell_pos.y);
        }

        if (key == GLFW_KEY_F) {
            show_fps = !show_fps;
        }

        if (key == GLFW_KEY_R) {
            auto &progress = registry.progresses.components[0];
            if ((progress.is_game_over && !progress.is_tutorial) ||
                (progress.is_tutorial && progress.is_game_over && !progress.has_beat_level))
                trigger_restart();
        }

        // TODO: Change progression behaviour in the future
        if (key == GLFW_KEY_ENTER) {
            auto &progress = registry.progresses.components[0];
            try_progress_tutorial();
            // if (!progress.is_tutorial && progress.has_beat_level && !progress.is_game_over)
            //    load_next_level();
        }

#pragma region DEBUG KEYS

#define ENABLE_LOAD_TUTORIAL
#define ENABLE_SKIP_WAVE
#define ENABLE_USER_INPUT_PAUSE_GAME
        // #define DEBUG_UPGRADE_INPUTS

#ifdef DEBUG_UPGRADE_INPUTS
        {
            auto &progress = registry.progresses.components[0];
            if (progress.is_upgrade_menu_called && action == GLFW_PRESS) {
                if (key == GLFW_KEY_1)
                    upgrade_system->try_upgrade_player(UPGRADE_TYPE::ATTACK);

                if (key == GLFW_KEY_2)
                    upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_AMMO);

                if (key == GLFW_KEY_3)
                    upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_HP);
            }

            if (key == GLFW_KEY_Z && action == GLFW_PRESS)
                upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_HP);

            if (key == GLFW_KEY_X && action == GLFW_PRESS)
                upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_AMMO);

            if (key == GLFW_KEY_C && action == GLFW_PRESS)
                upgrade_system->try_upgrade_player(UPGRADE_TYPE::ATTACK);
        }
#endif
        // #undef DEBUG_UPGRADE_INPUTS

        // Weapon switching
        if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
            Entity player = registry.players.entities[0];
            if (registry.weaponStates.has(player)) {
                WeaponState &ws = registry.weaponStates.get(player);
                ws.current_weapon = WeaponType::GUN_1;

                // reset turret to original color
                if (!registry.turrets.entities.empty()) {
                    Entity turret = registry.turrets.entities[0];
                    if (registry.tints.has(turret)) {
                        registry.tints.remove(turret);
                    }
                }
            }
        }
        if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
            Entity player = registry.players.entities[0];
            if (registry.weaponStates.has(player)) {
                WeaponState &ws = registry.weaponStates.get(player);
                ws.current_weapon = WeaponType::GUN_2;

                // make turret red
                if (!registry.turrets.entities.empty()) {
                    Entity turret = registry.turrets.entities[0];
                    if (registry.tints.has(turret)) {
                        registry.tints.get(turret).tint = vec3(1.0f, 0.1f, 0.1f);
                    } else {
                        registry.tints.emplace(turret, vec3(1.0f, 0.1f, 0.1f));
                    }
                }
            }
        }

#ifdef ENABLE_LOAD_TUTORIAL
        if (key == GLFW_KEY_T) {
            auto &progress = registry.progresses.components[0];
            progress.is_tutorial = true;
            trigger_restart();
        }
#endif

#ifdef ENABLE_SKIP_WAVE
        if (key == GLFW_KEY_Q) {
            auto &progress = registry.progresses.components[0];
            if (progress.is_tutorial) {
                progress.is_tutorial_step_called = false;
                progress.is_paused = false;
            }
            progress.trigger_skip_wave = true;

            for (Entity e : registry.enemies.entities)
                registry.remove_all_components_of(e);

            Entity player = registry.players.entities[0];
            Health &health = registry.healths.get(player);
            health.hp = health.max_hp;
            FireControl &fc = registry.fireControls.get(player);
            fc.ammo = fc.max_ammo;
        }

        if (key == GLFW_KEY_Z) {
            for (auto &wallet : registry.wallets.components)
                wallet.crackers += 5;
        }
#endif

#ifdef ENABLE_USER_INPUT_PAUSE_GAME
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            auto &progress = registry.progresses.components[0];
            if (progress.is_upgrade_menu_called) {
                upgrade_system->close_upgrade_menu();
            } else if (!progress.is_tutorial_step_called) {
                progress.is_paused = !progress.is_paused;
            }
        }
#endif
    }

    // Log debug info
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        registry.list_all_components();
    }

    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        persist_file.save();
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        persist_file.load();
    }

    // Log persistence info
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        Persistence &persist = registry.persistences.components[0];
        spdlog::info("Level: {}, Wave: {}", persist.level, persist.wave);
        spdlog::info("Wallet: {}", persist.wallet.crackers);

        for (int i = 0; i < UPGRADE_TYPE::UPGRADE_COUNT; i++) {
            spdlog::info("Upgrade {}: {}", i, persist.upgrades.upgrade_instances[i].level);
        }
    }

#pragma endregion DEBUG KEYS
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    Entity e = registry.players.entities[0];
    MouseInputState &mi = registry.mouseInputs.get(e);
    mi.mouse_pos = mouse_position;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
    Entity e = registry.players.entities[0];
    MouseInputState &mi = registry.mouseInputs.get(e);

    ImGuiIO &io = ImGui::GetIO();
    if (action == GLFW_PRESS && !io.WantCaptureMouse) // Ignore if ImGui is focused
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mi.current_action = MouseActions::LeftClick;
            try_progress_tutorial();
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            mi.current_action = MouseActions::RightClick;
        }
    } else if (action == GLFW_RELEASE) {
        // Clear action on release
        if ((button == GLFW_MOUSE_BUTTON_LEFT && mi.current_action == MouseActions::LeftClick) ||
            (button == GLFW_MOUSE_BUTTON_RIGHT && mi.current_action == MouseActions::RightClick)) {
            mi.current_action = MouseActions::MouseActionsCount;
        }
    }
}

void WorldSystem::trigger_restart() { registry.progresses.components[0].trigger_restart = true; }

// Header
#include "systems/world/world_system.hpp"
#include "systems/world/world_init.hpp"

// stlib
#include "configs/wave_config.cpp"
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
    // Destroy music components
    // if (background_music != nullptr)
    // 	Mix_FreeMusic(background_music);
    // if (invader_hit_sound != nullptr)
    // 	Mix_FreeChunk(invader_hit_sound);
    // if (tower_hit_sound != nullptr)
    // 	Mix_FreeChunk(tower_hit_sound);
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

    /*background_music = Mix_LoadMUS(get_path(FilePath::AUDIO, "music.wav").c_str());
    invader_hit_sound = Mix_LoadWAV(get_path(FilePath::AUDIO, "invader_hit.wav").c_str());
    tower_hit_sound = Mix_LoadWAV(get_path(FilePath::AUDIO, "tower_hit.wav").c_str());

    if (background_music == nullptr || invader_hit_sound == nullptr || tower_hit_sound == nullptr) {
            fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
                    get_path(FilePath::AUDIO, "music.wav").c_str(),
                    get_path(FilePath::AUDIO, "invader_hit.wav").c_str(),
                    get_path(FilePath::AUDIO, "tower_hit.wav").c_str());
            return false;
    }*/

    return true;
}

void WorldSystem::start_wave(Progress &progress) {
    auto config = WAVE_CONFIGS[progress.wave];
    progress.remaining_enemies = config.enemies.size();
    vec2 pos;

    if (registry.players.entities.size() > 0) {
        // // Teleport player to a random position each wave
        // Entity player = registry.players.entities[0];
        // registry.motions.get(player).position = tilemap_system->get_random_position(SpawnTarget::PLAYER);
    } else {
        // Create player entity
        pos = tilemap_system->get_random_position(SpawnTarget::PLAYER);
        Entity player = createPlayer(renderer, pos);

        // Create camera
        camera_system->init(tilemap_system->get_current_map());
    }

    for (auto enemy : config.enemies) {
        if (enemy == ENEMY_TYPE::MOTHER) {
            if (registry.tilemaps.has(tilemap_system->get_current_map())) {
                Entity sp_e = Entity();
                BezierPath &path = registry.tilemaps.get(tilemap_system->get_current_map()).patrol_loop[0];
                pos = path.p0;
            }
        } else {
            pos = tilemap_system->get_random_position(SpawnTarget::ENEMY);
        }

        Entity sp_e = Entity();
        registry.spawnRequests.insert(sp_e, {pos, 2000.f, enemy});
        createEffectSpawn(pos);
        
        /*if (enemy == ENEMY_TYPE::MOTHER) {
            if (registry.tilemaps.has(tilemap_system->get_current_map())) {
                BezierPath &path = registry.tilemaps.get(tilemap_system->get_current_map()).patrol_loop[0];
                Entity leader = createEnemy(renderer, path.p0, enemy);
                for (int i = 1; i < 7; i++) {
                    createFollower(renderer, path.p0 + vec2(TILE_SIZE_PX * i, 0.f), leader);
                    createEffectBubbles(path.p0);
                    progress.remaining_enemies += 1;
                }
            }
        } else {
            pos = tilemap_system->get_random_position(SpawnTarget::ENEMY);
            createEnemy(renderer, pos, enemy);
        }*/ 
    }
}

void WorldSystem::clear_level_objects() {
    for (auto entity_list : entities_to_clear)
        for (auto entity : *entity_list)
            registry.remove_all_components_of(entity);
}

void WorldSystem::load_level_tutorial(Progress &progress) {
    progress.is_tutorial = true;
    progress.is_tutorial_step_called = false;
    progress.tutorial_step_id = Tutorial::MOVEMENT;
    progress.tutorial_steps = TUTORIAL_STEPS;

    Entity player = createPlayer(renderer, {250.f, 1750.f});
    auto &player_motion = registry.motions.get(player);
    player_motion.angle = -M_PI / 2;
    auto &player_fc = registry.fireControls.get(player);
    player_fc.ammo = 0;

    createEnemy(renderer, {150.f, 200.f}, ENEMY_TYPE::GUARD);
    createEnemy(renderer, {850.f, 200.f}, ENEMY_TYPE::GUARD);
    createEnemy(renderer, {500.f, 400.f}, ENEMY_TYPE::GUARD);
    progress.remaining_enemies = registry.enemies.size();

    // Set enemy health to something low for tutorial
    for (Entity e : registry.enemies.entities) {
        Health &h = registry.healths.get(e);
        h.hp = 50;
        h.max_hp = 50;
    }

    createBubbler(renderer, {625.f, 875.f});
    createBubbler(renderer, {425.f, 400.f});

    createMine(renderer, {450.f, 1150.f});
    createPowerUp(renderer, {900.f, 1300.f}, POWER_UP_TYPE::HEAL);

    createTutorialTrigger(player_motion.position, vec2(50.f, 50.f), Tutorial::MOVEMENT);
    createTutorialTrigger(vec2(250.f, 1125.f), vec2(50.f, 350.f), Tutorial::HAZARDS);
    createTutorialTrigger(vec2(600.f, 1150.f), vec2(50.f, 450.f), Tutorial::HEALING);
    createTutorialTrigger(vec2(750.f, 875.f), vec2(400.f, 150.f), Tutorial::AMMUNITION);
    createTutorialTrigger(vec2(750.f, 725.f), vec2(400.f, 50.f), Tutorial::COMBAT);

    camera_system->init(tilemap_system->get_current_map()); // Must be done last, requires player to exist
}

void WorldSystem::load_level(Progress &progress) {
    progress.is_tutorial = false;

    tilemap_system->init_spawn_points();
    auto config = LEVEL_CONFIGS[progress.level];
    vec2 pos;

    for (auto item : config.hazards) {
        pos = tilemap_system->get_random_position(SpawnTarget::HAZARD);
        if (item == HAZARD_TYPE::MINE) {
            createMine(renderer, pos);
        } else if (item == HAZARD_TYPE::ANEMONE) {
            createAnemone(renderer, pos);
        }
    }

    for (auto bubbler : config.bubblers) {
        pos = tilemap_system->get_random_position(SpawnTarget::AMMO);
        createBubbler(renderer, pos);
    }

    for (auto item : config.powerups) {
        pos = tilemap_system->get_random_position(SpawnTarget::POWERUP);
        createPowerUp(renderer, pos, item);
    }

    for (auto bell : config.bells) {
        pos = tilemap_system->get_random_position(SpawnTarget::DISTRACTION);
        createBell(renderer, pos, bell);
    }
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

        load_next_level();
    }
}

void WorldSystem::init_tutorial(RenderSystem *renderer_arg, CameraSystem *camera_arg, TileMapSystem *tilemap_arg,
                                UpgradeSystem *upgrade_system) {
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;
    this->tilemap_system = tilemap_arg;
    this->upgrade_system = upgrade_system;

    // Set up world level
    tilemap_system->init();
    tilemap_system->load_map(MapType::TUTORIAL_MAP);

    auto e = Entity("Progress");
    auto &progress = registry.progresses.emplace(e);
    progress.level = -1;
    load_level_tutorial(progress);

    glm::vec3 font_color = glm::vec3(0.f, 1.f, 0.f);
    createText("Tutorial", vec2(WINDOW_WIDTH_PX / 2 - 50, WINDOW_HEIGHT_PX - 48), 1.f, font_color, glm::mat4(1.f));
}

void WorldSystem::init(RenderSystem *renderer_arg, CameraSystem *camera_arg, TileMapSystem *tilemap_arg,
                       UpgradeSystem *upgrade_system) {
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;
    this->tilemap_system = tilemap_arg;
    this->upgrade_system = upgrade_system;

    Progress *progress_p;
    if (registry.progresses.size() == 0) {
        auto e = Entity("Progress");
        progress_p = &registry.progresses.emplace(e);
        progress_p->level = 0;
    } else {
        progress_p = &registry.progresses.components[0];
    }
    Progress &progress = *progress_p;

    // Set up world level
    tilemap_system->init();
    tilemap_system->load_map(LEVEL_CONFIGS[progress.level].map_type);

    load_level(progress);
    start_wave(progress);

    // ! FOR TESTING !
    glm::vec3 font_color = glm::vec3(0.f, 1.f, 0.f);
    createText("Level " + std::to_string(progress.level + 1) + " - Wave " + std::to_string(progress.wave + 1),
               vec2(WINDOW_WIDTH_PX / 2 - 100, WINDOW_HEIGHT_PX - 48), 1.f, font_color, glm::mat4(1.f));

    /* Uncomment to use old testing entities
    createBell(renderer, { 300.f, 400.f }, 80.f);
    createBell(renderer, { 700.f, 200.f }, 80.f);

    // ! FOR TESTING ! NEED TO FIX COLOUR INDICATORS (maybe set an init colour)
    // createOverTimeEffect<Damage>(player, 1000.f, 5000.f, 10);
    // createOverTimeEffect<Heal>(player, 1000.f, 5000.f, 10);

    ENEMY_TYPE type = ENEMY_TYPE::URCHIN; // TESTING: change this out to test types
    Entity enemy1 = createEnemy(renderer, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 5 }, type);

    // ! FOR TESTING !
    createPowerUp(renderer, {500.f, 500.f}, POWER_UP_TYPE::HEAL);
    createPowerUp(renderer, {700.f, 500.f}, POWER_UP_TYPE::SPEED_MULTIPLIER);
    createPowerUp(renderer, {600.f, 500.f}, POWER_UP_TYPE::DAMAGE_MULTIPLIER);

    // ! FOR TESTING !
    createMine(renderer, {500.f, 400.f});
    createAnemone(renderer, {500.f, 600.f});

    // start playing background music indefinitely
    //Commented out duo to compiler errors
    spdlog::info("Starting music...");
    Mix_PlayMusic(background_music, -1);*/
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

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

    if (progress.remaining_enemies == 0) {
        spdlog::info("Wave {} complete, starting next wave.", progress.wave + 1);
        progress.wave += 1;

        if (progress.is_tutorial) {
            if (registry.texts.components.size() > 0)
                registry.texts.components[0].text = "Tutorial";

            progress.is_game_over = true;
            progress.has_beat_level = true;
            progress.wave = 0;
            progress.level = -1;

            // TODO: Also change this to a GUI overlay
            Entity e = Entity("Overlay");
            registry.overlays.emplace(e);
            auto &motion = registry.motions.emplace(e);
            motion.position = registry.motions.get(registry.cameras.entities[0]).position;
            motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);

            registry.renderRequests.insert(e, {TEXTURE_ASSET_ID::TEXTURE_TUTORIAL_COMPLETE_OVERLAY,
                                               EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE,
                                               RENDER_LAYER::OVERLAYS});
        } else {
            if (progress.wave == LEVEL_CONFIGS[progress.level].last_wave && progress.level < LEVEL_CONFIGS.size() - 1) {
                load_next_level();
            } else if (WAVE_CONFIGS.size() > progress.wave && progress.wave < LEVEL_CONFIGS[progress.level].last_wave) {
                // TODO: Maybe change this to a GUI overlay
                if (registry.texts.components.size() > 0)
                    registry.texts.components[0].text =
                        "Level " + std::to_string(progress.level + 1) + " - Wave " + std::to_string(progress.wave + 1);
                start_wave(progress);
            } else {
                progress.is_game_over = true;
                progress.has_beat_level = true;

                // TODO: Also change this to a GUI overlay
                Entity e = Entity("Overlay");
                registry.overlays.emplace(e);
                auto &motion = registry.motions.emplace(e);
                motion.position = registry.motions.get(registry.cameras.entities[0]).position;
                motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);

                registry.renderRequests.insert(e, {TEXTURE_ASSET_ID::TEXTURE_WIN_OVERLAY, EFFECT_ASSET_ID::TEXTURED,
                                                   GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::OVERLAYS});
            }
        }
    }

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

    for (Entity e : registry.spawnRequests.entities) {
        SpawnRequest &req = registry.spawnRequests.get(e);
        req.timer_ms -= elapsed_ms_since_last_update;
        if (req.timer_ms <= 0) {
            createEnemy(e, renderer, req.position, req.type);
            createEffectSmoke(req.position);
            if (req.type == ENEMY_TYPE::MOTHER) {
                for (int i = 1; i < 7; i++) {
                    createFollower(renderer, req.position + vec2(TILE_SIZE_PX * i, 0.f), e);
                    progress.remaining_enemies += 1;
                }
            } 

            registry.spawnRequests.remove(e);
        }
    }

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

void WorldSystem::handle_expiration(float elapsed_ms) {
    for (Entity e : registry.expirations.entities) {
        Expiration &exp = registry.expirations.get(e);
        exp.lifespan_ms -= elapsed_ms;

        if (exp.lifespan_ms <= 0) {
            spdlog::debug("{} has expired", e.name_id());
            registry.remove_all_components_of(e);
        }
    }

    for (Entity e : registry.tints.entities) {
        Tint &tint = registry.tints.get(e);

        if (tint.life_ms == -99.f)
            continue;

        tint.life_ms -= elapsed_ms;
        if (tint.life_ms <= 0)
            registry.tints.remove(e);
    }
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

    // exit game w/ ESC
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        close_window();
    }

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
                restart_game();
        }

        // TODO: Change progression behaviour in the future
        if (key == GLFW_KEY_ENTER) {
            auto &progress = registry.progresses.components[0];
            try_progress_tutorial();
            if (!progress.is_tutorial && progress.has_beat_level && !progress.is_game_over)
                load_next_level();
        }

#pragma region DEBUG KEYS

// #define ENABLE_LOAD_TUTORIAL
// #define ENABLE_SKIP_WAVE
#define ENABLE_USER_INPUT_PAUSE_GAME
#define DEBUG_UPGRADE_INPUTS

#ifdef DEBUG_UPGRADE_INPUTS
        if (key == GLFW_KEY_Z && action == GLFW_PRESS)
            upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_HP);

        if (key == GLFW_KEY_X && action == GLFW_PRESS)
            upgrade_system->try_upgrade_player(UPGRADE_TYPE::MAX_AMMO);

        if (key == GLFW_KEY_C && action == GLFW_PRESS)
            upgrade_system->try_upgrade_player(UPGRADE_TYPE::ATTACK);
#endif
#undef DEBUG_UPGRADE_INPUTS

#ifdef ENABLE_LOAD_TUTORIAL
        if (key == GLFW_KEY_T) {
            auto &progress = registry.progresses.components[0];
            progress.is_tutorial = true;
            restart_game();
        }
#endif

#ifdef ENABLE_SKIP_WAVE
        if (key == GLFW_KEY_Q) {
            auto &progress = registry.progresses.components[0];
            if (progress.is_tutorial) {
                progress.is_tutorial_step_called = false;
                progress.is_paused = false;
            }

            for (Entity e : registry.enemies.entities)
                registry.remove_all_components_of(e);
            progress.remaining_enemies = 0;

            Entity player = registry.players.entities[0];
            Health &health = registry.healths.get(player);
            health.hp = health.max_hp;
            FireControl &fc = registry.fireControls.get(player);
            fc.ammo = fc.max_ammo;
        }
#endif

#ifdef ENABLE_USER_INPUT_PAUSE_GAME
        if (key == GLFW_KEY_P) {
            auto &progress = registry.progresses.components[0];
            if (!progress.is_tutorial_step_called)
                progress.is_paused = !progress.is_paused;

            if (progress.is_upgrade_menu_called)
                progress.is_upgrade_menu_called = false;
        }
#endif
    }

    // Log debug info
    if (key == GLFW_KEY_L) {
        registry.list_all_components();
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
        }
    } else {
        mi.current_action = MouseActions::MouseActionsCount;
    }
}

void WorldSystem::restart_game() {
    bool is_tutorial = registry.progresses.components[0].is_tutorial;

    particle_system.clear_all_particles();
    registry.clear_all_components();

    if (is_tutorial) {
        init_tutorial(renderer, camera_system, tilemap_system, upgrade_system);
    } else {
        init(renderer, camera_system, tilemap_system, upgrade_system);
    }
}

void WorldSystem::load_next_level() {
    Progress progress = registry.progresses.components[0];

    progress.is_tutorial = false;
    progress.is_tutorial_step_called = false;
    progress.has_beat_level = false;
    progress.is_game_over = false;
    progress.level++;

    if (progress.level >= LEVEL_CONFIGS.size()) {
        close_window();
        return;
    }

    progress.is_upgrade_menu_called = LEVEL_CONFIGS[progress.level].upgrade_screen_before;
    progress.is_paused = progress.is_upgrade_menu_called;

    int hp = registry.healths.get(registry.players.entities[0]).hp;
    int ammo = registry.fireControls.get(registry.players.entities[0]).ammo;

    particle_system.clear_all_particles();
    registry.clear_all_components();
    Entity e = Entity();
    registry.progresses.insert(e, progress);
    init(renderer, camera_system, tilemap_system, upgrade_system);

    registry.healths.get(registry.players.entities[0]).hp = hp;
    registry.fireControls.get(registry.players.entities[0]).ammo = ammo;
}

// Header
#include "systems/world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

#include "configs/wave_config.cpp"
#include "systems/particle_system.hpp"
#include "systems/physics_system.hpp"

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
        pos = tilemap_system->get_random_position(SpawnTarget::ENEMY);
        createEnemy(renderer, pos, enemy);
    }
}

void WorldSystem::load_level_tutorial(Progress& progress) {
    vec2 spawn_pos = {200.f, 1500.f};
    Entity player = createPlayer(renderer, {250.f,  1750.f});
    auto &player_motion = registry.motions.get(player);
    player_motion.angle = -M_PI / 2;
    auto &player_fc = registry.fireControl.get(player);
    player_fc.ammo = 0;

    Entity enemy_1 = createEnemy(renderer, {150.f, 200.f}, ENEMY_TYPE::FOLLOWER);
    Entity enemy_2 = createEnemy(renderer, {850.f, 200.f}, ENEMY_TYPE::FOLLOWER);
    Entity enemy_3 = createEnemy(renderer, {500.f, 400.f}, ENEMY_TYPE::FOLLOWER);

    Entity bubbler_1 = createBubbler(renderer, {650.f, 900.f});
    Entity bubbler_2 = createBubbler(renderer, {425.f, 400.f});
    progress.remaining_enemies = 3;

    camera_system->init(tilemap_system->get_current_map()); // Must be done last, requires player to exist
}

void WorldSystem::load_level(Progress &progress) {
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

void WorldSystem::init_tutorial(RenderSystem* renderer_arg, CameraSystem* camera_arg, TileMapSystem* tilemap_arg)
{
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;
    this->tilemap_system = tilemap_arg;

    // Set up world level
    tilemap_system->init();
    tilemap_system->load_map(MapType::TUTORIAL_MAP);

    auto e = Entity("Progress");
    auto &progress = registry.progress.emplace(e);
    load_level_tutorial(progress);

    glm::vec3 font_color = glm::vec3(0.f, 1.f, 0.f);
    glm::vec2 position = vec2(2.f, WINDOW_HEIGHT_PX - 24);
    createText("FPS: 0", position, 1.f, font_color);
    createText("Wave: 0", vec2(WINDOW_WIDTH_PX / 2 - 50, WINDOW_HEIGHT_PX - 48), 1.f, font_color, glm::mat4(1.f));
}

void WorldSystem::init(RenderSystem *renderer_arg, CameraSystem *camera_arg, TileMapSystem *tilemap_arg) {
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;
    this->tilemap_system = tilemap_arg;

    // Set up world level
    tilemap_system->init();
    tilemap_system->load_map(MapType::TEST_MAP2);

    // assert(registry.progress.size() == 0 && "Progress component should have been deleted prior to reinitialization!");

    Progress *progress_p;
    if (registry.progress.size() == 0)
    {
        auto e = Entity("Progress");
        progress_p = &registry.progress.emplace(e);
    } else {
        progress_p = &registry.progress.components[0];
    }
    Progress &progress = *progress_p;

    load_level(progress);
    start_wave(progress);

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

    // ! FOR TESTING !
    glm::vec3 font_color = glm::vec3(0.f, 1.f, 0.f);
    glm::vec2 position = vec2(2.f, WINDOW_HEIGHT_PX - 24);
    createText("FPS: 0", position, 1.f, font_color);
    createText("Wave: 0", vec2(WINDOW_WIDTH_PX / 2 - 50, WINDOW_HEIGHT_PX - 48), 1.f, font_color, glm::mat4(1.f));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

    // Updating window title
    std::stringstream title_ss;
    title_ss << GAME_NAME;
    glfwSetWindowTitle(window, title_ss.str().c_str());

    // Updating FPS
    // ! FOR TESTING !
    fps = 1000.0f / elapsed_ms_since_last_update;
    if (registry.text.components.size() > 0)
        registry.text.components[0].text = "FPS: " + std::to_string((int)fps);

    Progress &progress = registry.progress.components[0];
    if (progress.game_over == true) {
        ScreenState &screen = registry.screenStates.components[0];
        if (screen.darken_screen_factor < 0.9f) {
            screen.darken_screen_factor += elapsed_ms_since_last_update / 1000.f;
        }
        return false;
    }

    if (progress.remaining_enemies == 0) {
        spdlog::info("Wave {} complete, starting next wave.", progress.wave);
        progress.wave += 1;

        // ! FOR TESTING !
        if (registry.text.components.size() > 1)
            registry.text.components[1].text = "Wave: " + std::to_string(progress.wave);

        if (WAVE_CONFIGS.size() > progress.wave && progress.wave < LEVEL_CONFIGS[progress.level].last_wave) {
            start_wave(progress);
        } else {
            progress.game_over = true;
            progress.beat_level = true;
            Entity e = Entity("Overlay");
            registry.overlays.emplace(e);
            auto &motion = registry.motions.emplace(e);
            motion.position = registry.motions.get(registry.camera.entities[0]).position;
            motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);
            registry.renderRequests.insert(e, {TEXTURE_ASSET_ID::TEXTURE_WIN_OVERLAY, EFFECT_ASSET_ID::TEXTURED,
                                               GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::OVERLAYS});
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

    /*if (registry.distracted.entities.size() > 0)
    {
            Entity evt_e = registry.distracted.entities[0];
            Distracted& evt = registry.distracted.get(evt_e);
            evt.remaining_ms -= elapsed_ms_since_last_update;

            if (evt.remaining_ms <= 0.f)
            {
                    registry.remove_all_components_of(evt_e);
                    spdlog::info("[Bell] {} expired", evt_e.name_id());
            }
    }*/

    // Create bullet entities requested by enemies in AISystem
    for (BulletRequest req : registry.bullet_requests.components) {
        createProjectile(req.position, req.angle, req.attack_pattern, req.owner);
    }
    registry.bullet_requests.clear();

    // Remove debug info from the last step
    while (registry.debugComponents.entities.size() > 0)
        registry.remove_all_components_of(registry.debugComponents.entities.back());

    return true;
}

void WorldSystem::handle_expiration(float elapsed_ms) {
    for (Entity e : registry.expiration.entities) {
        Expiration &exp = registry.expiration.get(e);
        exp.lifespan_ms -= elapsed_ms;

        if (exp.lifespan_ms <= 0) {
            spdlog::debug("{} has expired", e.name_id());
            registry.remove_all_components_of(e);
        }
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
    assert(registry.keyboardInput.components.size() == 1);
    Entity playerEntity = registry.players.entities[0];
    KeyboardInputState &ps = registry.keyboardInput.get(playerEntity);

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
                spdlog::debug("[Bell] No player entity found");
                return;
            }

            Entity player = registry.players.entities[0];
            if (!registry.motions.has(player)) {
                spdlog::debug("[Bell] {} has no Motion", player.name_id());
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
                spdlog::debug("[Bell] No bell in the world");
                return;
            }

            Bell &bell = registry.bells.get(best_bell);
            float radius = bell.interact_radius_px;
            float radius2 = radius * radius;

            if (best_dist2 > radius2) {
                float dist = sqrtf(best_dist2);
                spdlog::debug("[Bell] {} too far to interact. dist={:.2f} (need <= {:.2f})", best_bell.name_id(), dist,
                              radius);
                return;
            }

            if (bell.cooldown_timer > 0.f) {
                spdlog::debug("[Bell] {} on cooldown: {:.2f}ms", best_bell.name_id(), bell.cooldown_timer);
                return;
            }

            vec2 bell_pos = registry.motions.get(best_bell).position;

            for (Entity enemy : registry.enemies.entities) {
                Motion &enemy_motion = registry.motions.get(enemy);
                if (!registry.distracted.has(enemy) &&
                    entity_in_radius(bell_pos - enemy_motion.position, enemy_motion.scale, bell.distraction_radius)) {
                    if (registry.pursuit.has(enemy)) {
                        registry.pursuit.remove(enemy);
                    }
                    registry.distracted.insert(enemy, {best_bell, bell.distraction_ms});
                }
            }

            bell.cooldown_timer = bell.cooldown_ms;

            spdlog::debug("[Bell] {} triggered at ({:.2f}, {:.2f})", best_bell.name_id(), bell_pos.x, bell_pos.y);
        }

        if (key == GLFW_KEY_R) {
            auto &progress = registry.progress.components[0];
            if (progress.game_over) {
                restart_game();
            }
        }

        if (key == GLFW_KEY_ENTER) {
            auto &progress = registry.progress.components[0];
            if (progress.beat_level)
                load_next_level();
        }
    }

    // Debug
    if (key == GLFW_KEY_P) {
        registry.list_all_components();
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    Entity e = registry.players.entities[0];
    MouseInputState &mi = registry.mouseInput.get(e);
    mi.mouse_pos = mouse_position;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
    Entity e = registry.players.entities[0];
    MouseInputState &mi = registry.mouseInput.get(e);

    ImGuiIO &io = ImGui::GetIO();
    if (action == GLFW_PRESS && !io.WantCaptureMouse) // Ignore if ImGui is focused
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mi.current_action = MouseActions::LeftClick;
        }
    } else {
        mi.current_action = MouseActions::MouseActionsCount;
    }
}

void WorldSystem::restart_game() {
    particle_system.clear_all_particles();
    registry.clear_all_components();
    init_tutorial(renderer, camera_system, tilemap_system);
}

void WorldSystem::load_next_level() {
    Progress progress = registry.progress.components[0];
    progress.level++;

    if (progress.level >= LEVEL_CONFIGS.size()) {
        close_window();
        return;
    }

    progress.beat_level = false;
    progress.game_over = false;
    progress.wave = 0;
    int hp = registry.health.get(registry.players.entities[0]).hp;

    particle_system.clear_all_particles();
    registry.clear_all_components();
    Entity e = Entity();
    registry.progress.insert(e, progress);
    init(renderer, camera_system, tilemap_system);

    registry.health.get(registry.players.entities[0]).hp = hp;
}
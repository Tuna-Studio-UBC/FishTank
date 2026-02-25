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
#include "systems/tilemap_system.hpp"

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
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE); // GLFW 3.3+

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

void WorldSystem::init_free_tiles() {
    free_tiles.clear();
    TileMap &tilemap = registry.tilemaps.get(current_map);
    TileSet &tileset = registry.tilesets.get(current_map);

    for (unsigned int y = 0; y < tilemap.height; y++) {
        for (unsigned int x = 0; x < tilemap.width; x++) {
            int tile_id = tilemap.tile_ids[x + y * tilemap.width];

            bool is_border = (y == 0 || y == tilemap.height - 1 || x == 0 || x == tilemap.width - 1);
            bool is_empty = is_empty_tile(tile_id);

            if (is_empty && !is_border) {
                free_tiles.push_back({x, y});
            }
        }
    }
}

vec2 WorldSystem::get_random_position() {
    int idx = (int)(uniform_dist(rng) * free_tiles.size()) - 1;
    vec2 tile = free_tiles[idx];
    free_tiles[idx] = free_tiles.back();
    free_tiles.pop_back();
    return vec2(tile.x * TILE_WIDTH_PX + (TILE_WIDTH_PX / 2), tile.y * TILE_HEIGHT_PX + (TILE_HEIGHT_PX / 2));
}

void WorldSystem::start_wave(Progress &progress) {
    auto config = WAVE_CONFIGS[progress.wave];
    progress.remaining_enemies = config.enemies.size();

    for (auto enemy : config.enemies) {
        vec2 pos = get_random_position();
        createEnemy(renderer, pos, enemy);
    }
}

void WorldSystem::load_level(Progress &progress) {
    init_free_tiles();
    auto config = LEVEL_CONFIGS[progress.level];
    vec2 pos;

    for (auto item : config.hazards) {
        pos = get_random_position();
        if (item == HAZARD_TYPE::MINE) {
            createMine(renderer, pos);
        } else if (item == HAZARD_TYPE::ANEMONE) {
            createAnemone(renderer, pos);
        }
    }

    for (auto item : config.powerups) {
        pos = get_random_position();
        createPowerUp(renderer, pos, item);
    }

    for (auto bell : config.bells) {
        pos = get_random_position();
        createBell(renderer, pos, bell);
    }
}

void WorldSystem::init(RenderSystem *renderer_arg, CameraSystem *camera_arg) {
    this->renderer = renderer_arg;
    this->camera_system = camera_arg;

    // Set up world level
    current_map = TileMapSystem::load_map();

    // Create player entity
    Entity player = createPlayer(renderer, {WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX / 2.0f});

    // Create camera
    camera_system->init(current_map);

    auto e = Entity("Progress");
    auto &progress = registry.progress.emplace(e);
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

    // walls
    TileMap &tilemap = registry.tilemaps.components[0];
    float arena_width = tilemap.width * TILE_SIZE_PX;
    float arena_height = tilemap.height * TILE_SIZE_PX;
    float wall_thickness = TILE_SIZE_PX;
    // top
    createWall(vec2(arena_width / 2.f, wall_thickness / 2.f), vec2(arena_width, wall_thickness));
    // bottom
    createWall(vec2(arena_width / 2.f, arena_height - wall_thickness / 2.f), vec2(arena_width, wall_thickness));
    // left
    createWall(vec2(wall_thickness / 2.f, arena_height / 2.f), vec2(wall_thickness, arena_height));
    // right
    createWall(vec2(arena_width - wall_thickness / 2.f, arena_height / 2.f), vec2(wall_thickness, arena_height));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

    // Updating window title
    std::stringstream title_ss;
    title_ss << GAME_NAME;
    glfwSetWindowTitle(window, title_ss.str().c_str());

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

        if (WAVE_CONFIGS.size() > progress.wave) {
            if (progress.wave > LEVEL_CONFIGS[progress.level].last_wave) {
                progress.level += 1;
                load_level(progress);
            }
            start_wave(progress);
        } else {
            progress.game_over = true;

            Entity e = Entity("Overlay");
            registry.overlays.emplace(e);
            auto &motion = registry.motions.emplace(e);
            motion.position = registry.motions.get(registry.camera.entities[0]).position;
            motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);
            registry.renderRequests.insert(
                e, {TEXTURE_ASSET_ID::TEXTURE_WIN_OVERLAY, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
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
            if (registry.progress.components[0].game_over) {
                restart_game();
            }
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
    init(renderer, camera_system);
}

#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "systems/audio/audio_system.hpp"
#include "systems/player/camera_system.hpp"
#include "systems/rendering/render_system.hpp"
#include "systems/world/tilemap_system.hpp"

// Container for all our entities and game logic.
// Individual rendering / updates are deferred to the update() methods.
class WorldSystem {
  public:
    WorldSystem();

    // creates main window
    GLFWwindow *create_window();

    // starts and loads music and sound effects
    bool start_and_load_sounds();

    // call to close the window
    void close_window();

    void restart_game();

    // starts the game
    void init_tutorial(RenderSystem *renderer_arg, CameraSystem *camera_arg, TileMapSystem *tilemap_arg);
    void init(RenderSystem *renderer, CameraSystem *camera_system, TileMapSystem *tilemap_system);

    // releases all associated resources
    ~WorldSystem();

    // steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // check for expired entities and remove them
    void handle_expiration(float elapsed_ms);

    void respawn_player();

    float get_fps() const {
        float fps = 0.f;
        for (float frame_fps : fps_avg)
            fps += frame_fps;
        return fps / fps_avg.size();
    }
    bool get_show_fps() const { return show_fps; }

    // should the game be over ?
    bool is_over() const;

  private:
    void start_wave(Progress &progress);
    void clear_level_objects();
    void load_level_tutorial(Progress &progress);
    void load_level(Progress &progress);
    void load_next_level();

    // input callback functions
    void on_key(int key, int, int action, int mod);
    void on_mouse_move(vec2 pos);
    void on_mouse_button_pressed(int button, int action, int mods);

    // OpenGL window handle
    GLFWwindow *window;

    // Game state
    TileMapSystem *tilemap_system;
    RenderSystem *renderer;
    CameraSystem *camera_system;

    std::vector<std::vector<Entity> *> entities_to_clear = {&registry.enemies.entities, &registry.hazard.entities,
                                                            &registry.bullet.entities,  &registry.pickUp.entities,
                                                            &registry.bells.entities,   &registry.bubblers.entities};

    // grid
    std::vector<Entity> grid_lines;

    // FPS
    int fps_index = 0;
    std::array<float, 100> fps_avg = {30.f}; // Store last 100 frames
    bool show_fps = false;

    // music references
    // Mix_Music* background_music;
    // Mix_Chunk* invader_hit_sound;
    // Mix_Chunk* tower_hit_sound;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    // audio
    AudioSystem audio_system;
};

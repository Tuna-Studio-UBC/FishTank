#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "systems/camera_system.hpp"
#include "systems/render_system.hpp"
#include "systems/tilemap_system.hpp"

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
    void init(RenderSystem *renderer, CameraSystem *camera_system, TileMapSystem *tilemap_system);

    // releases all associated resources
    ~WorldSystem();

    // steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // check for expired entities and remove them
    void handle_expiration(float elapsed_ms);

    void respawn_player();

    // should the game be over ?
    bool is_over() const;

  private:
    void start_wave(Progress &progress);
    void load_level(Progress &progress);

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

    // music references
    // Mix_Music* background_music;
    // Mix_Chunk* invader_hit_sound;
    // Mix_Chunk* tower_hit_sound;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};

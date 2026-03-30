#pragma once

#include "common.hpp"
#include "configs/wave_config.cpp"
#include "systems/player/camera_system.hpp"
#include "systems/rendering/particle_system.hpp"
#include "systems/world/world_init.hpp"
#include "systems/world/world_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class LevelSystem {

  public:
    void step(const float &elapsed_ms);

    LevelSystem(TileMapSystem *tilemap_system, RenderSystem *renderer, CameraSystem *camera_system, WorldSystem *ws)
        : tilemap_system(tilemap_system), renderer(renderer), camera_system(camera_system), world_system(ws) {};

    void load_tutorial();

    void restart_game();
    int get_current_wave() const;
    int get_total_waves_for_current_level() const;
    bool is_in_tutorial() const;

  private:
    int wave = -1;
    int level = -1;

    float time_since_last_wave = 0.f;
    int enemies_spawned_last_wave = 0;

    float advance_timer = 0.f;
    bool triggered_load_wave = false;
    bool triggered_load_level = false;

    TileMapSystem *tilemap_system;
    CameraSystem *camera_system;
    RenderSystem *renderer;
    WorldSystem *world_system;

    void trigger_load_next_wave();
    void trigger_load_next_level();
    void trigger_win(Progress &progress);

    // Set `should_persist=true` to persist what's in registry at the time
    void load_level(int level, bool should_persist = true);
    void load_wave(int wave);

    void load_next_level();
    void load_level_components();
    void load_next_wave();

    bool should_advance();

    void handle_timer(const float &elapsed_ms);
};

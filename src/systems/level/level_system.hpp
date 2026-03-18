#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "configs/wave_config.cpp"
#include "systems/world/world_init.hpp"
#include "systems/player/camera_system.hpp"
#include "systems/world/world_system.hpp"
#include "systems/rendering/particle_system.hpp"

class LevelSystem
{

public:
    void step(const float& elapsed_ms);

    LevelSystem(TileMapSystem *tilemap_system, RenderSystem *renderer, 
        CameraSystem *camera_system, WorldSystem *ws) : 
        tilemap_system(tilemap_system),
        renderer(renderer),
        camera_system(camera_system),
        world_system(ws)
    { };

    void load_tutorial();

    void restart_game();

private:
    int wave = -1;
    int level = -1;

    TileMapSystem *tilemap_system;
    CameraSystem *camera_system;
    RenderSystem *renderer;
    WorldSystem *world_system;

    void trigger_load_next_wave();
    void trigger_load_next_level();

    void load_level(int level);
    void load_wave(int wave);

    void load_next_level();
    void load_level_components();
    void load_next_wave();

    bool should_advance();


};
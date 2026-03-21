#pragma once
#include "pathfinding_system.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include <tinyECS/registry.hpp>

class AISystem {
  private:
    PathfindingSystem *ps = nullptr;
    void act(Entity e, float elapsed_ms, vec2 player_pos);
    void run_pursuit(Entity e, float elapsed_ms, vec2 player_pos);
    void run_idle(Entity e);
    void run_distracted(Entity e, float elapsed_ms, vec2 player_pos);
    void run_patrol(Entity e, float elapsed_ms);

  public:
    void init(PathfindingSystem *system);
    void step(float elapsed_ms);
    void switch_attack_pattern(Boss &boss, FireControl &fc, int next_pattern);
    void handle_bosses(float elapsed_ms);
    void handle_detection();
};

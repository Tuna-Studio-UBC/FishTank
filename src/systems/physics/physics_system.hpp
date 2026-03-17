#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include <unordered_map>
#include <vector>

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  public:
    void step(float elapsed_ms);

    // Debug: access wall spatial grid for visualization
    struct GridCell {
        int cx, cy;
        uint wall_count;
    };
    static std::vector<GridCell> getOccupiedWallCells();
    static float getWallGridCellSize();

    PhysicsSystem() {}
};

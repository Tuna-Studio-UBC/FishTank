#pragma once
#include <vector>
#include "../../common.hpp"
#include "../../tinyECS/components.hpp"
#include "../tilemap_system.hpp"

class PathfindingSystem {
  private:
    TileMapSystem *tms;
    Entity current_map;
    bool is_walkable(int x, int y);

  public:
    void init(TileMapSystem *system);
    bool is_out_of_bounds(ivec2 pos);
    bool walkable_bresenhams_line(ivec2 a, ivec2 b);
    std::vector<ivec2> prune_path(std::vector<ivec2> &path);
    std::vector<vec2> path_finding(const vec2 &start_world, const vec2 &goal_world);
};

#pragma once
#include <vector>
#include "../../common.hpp"
#include "../../tinyECS/components.hpp"

bool is_out_of_bounds(ivec2 pos);
bool walkable_bresenhams_line(ivec2 a, ivec2 b, const TileMap &map);
std::vector<ivec2> prune_path(std::vector<ivec2> &path, const TileMap &map);
std::vector<vec2> path_finding_bfs(const vec2 &start_world, const vec2 &goal_world);
std::vector<vec2> path_finding_a(const vec2 &start_world, const vec2 &goal_world);
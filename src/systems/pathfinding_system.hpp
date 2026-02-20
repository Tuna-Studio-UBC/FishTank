#pragma once
#include <vector>
#include "../common.hpp"

std::vector<vec2> path_finding_bfs(const vec2 &start_world, const vec2 &goal_world);
std::vector<vec2> path_finding_a(const vec2 &start_world, const vec2 &goal_world);
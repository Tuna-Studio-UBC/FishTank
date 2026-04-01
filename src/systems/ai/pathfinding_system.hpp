#pragma once
#include "common.hpp"
#include "systems/world/tilemap_system.hpp"
#include "tinyECS/components.hpp"
#include <vector>

class PathfindingSystem {
  private:
    TileMapSystem *tms;
    Entity current_map;
    // pathfinding helpers
    bool is_walkable(int x, int y);

    // Bezier helpers
    vec2 evaluate_cubic_bezier(const BezierPath &path, float t);

    // Boids helpers
    vec2 compute_alignment(Entity e, std::vector<Entity> &flock);
    vec2 compute_cohesion(Entity e, std::vector<Entity> &flock);
    vec2 compute_separation(Entity e, std::vector<Entity> &flock);
    vec2 compute_follow_force(Entity follower, Entity leader);
    vec2 avoid_walls(Entity e);
    vec2 compute_boid_force(Entity e, std::vector<Entity> &flock, Entity leader);

  public:
    void init(TileMapSystem *system);
    bool is_out_of_bounds(ivec2 pos);
    bool walkable_bresenhams_line(ivec2 a, ivec2 b);
    bool visible_bresenhams_line(ivec2 a, ivec2 b);
    std::vector<ivec2> prune_path(std::vector<ivec2> &path);
    std::vector<vec2> path_finding(const vec2 &start_world, const vec2 &goal_world);
    void update_leader(Entity e, Motion &motion, float elapsed_ms);
    void update_follower(Entity follower, Entity leader, float elapsed_ms);

    Entity get_current_map() const { return current_map; }
};

#pragma once

#include "pathfinding_system.hpp"
#include <common.hpp>
#include <tinyECS/registry.hpp>

// Bezier helpers
void generate_bezier(vec2 start_pos, vec2 forward_dir, BezierPath &curve);
vec2 evaluate_cubic_bezier(const BezierPath &curve, float t);

// Boids helpers
vec2 compute_alignment(Entity e, std::vector<Entity> &flock);
vec2 compute_cohesion(Entity e, std::vector<Entity> &flock);
vec2 compute_separation(Entity e, std::vector<Entity> &flock);
vec2 compute_boid_force(Entity e, std::vector<Entity> &flock);

// Update
void update_leader(Entity e, Motion &motion, float elapsed_ms);
void update_follower(Entity follower, Entity leader, float elapsed_ms);

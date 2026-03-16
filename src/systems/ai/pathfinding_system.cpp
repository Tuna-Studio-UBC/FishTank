#include "pathfinding_system.hpp"
#include "tinyECS/registry.hpp"
#include <queue>

void PathfindingSystem::init(TileMapSystem *system) {
    tms = system;
    current_map = tms->get_current_map();
}

#pragma region Pathing
const ivec2 four_directions[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
const ivec2 eight_directions[8] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

struct Node {
    int x, y;
    int g;
    int f;

    bool operator>(const Node &other) const { return f > other.f; }
};

int get_map_index(int width, int x, int y) { return x + (y * width); }

bool PathfindingSystem::is_out_of_bounds(ivec2 pos) {
    if (!registry.tilemaps.has(current_map)) {
        current_map = tms->get_current_map();
    }
    TileMap &map = registry.tilemaps.get(current_map);
    return pos.x < 0 || pos.y < 0 || pos.x >= (int)map.width || pos.y >= (int)map.height;
}

bool PathfindingSystem::is_walkable(int x, int y) {
    if (!registry.tilemaps.has(current_map)) {
        current_map = tms->get_current_map();
    }
    TileMap &map = registry.tilemaps.get(current_map);
    if (is_out_of_bounds(ivec2(x, y))) {
        return false;
    }

    int tile_id = map.tile_ids[x + y * map.width];
    return is_empty_tile(tile_id);
}

int h_cost(const ivec2 &a, const ivec2 &b) {
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);

    int min_d = std::min(dx, dy);
    int max_d = std::max(dx, dy);

    return (14 * min_d) + (10 * (max_d - min_d));
}

bool PathfindingSystem::walkable_bresenhams_line(ivec2 a, ivec2 b) {
    int x1 = a.x;
    int y1 = a.y;
    int x2 = b.x;
    int y2 = b.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;
        //int prev_x = x1;
        //int prev_y = y1;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }

        // modify Bresenham's to check if all neighbours walkable
        if (!is_walkable(x1 - 1, y1) || !is_walkable(x1, y1 - 1) || !is_walkable(x1, y1 + 1) ||
            !is_walkable(x1 + 1, y1)) {
            return false;
        }
        /* if (x1 != prev_x && y1 != prev_y) {
            if (!is_walkable(prev_x, y1) || !is_walkable(x1, prev_y)) {
                return false;
            }
        }*/
    }

    return true;
}

std::vector<ivec2> PathfindingSystem::prune_path(std::vector<ivec2> &path) {
    if (path.size() <= 2) {
        return path;
    }

    std::vector<ivec2> pruned_path;
    pruned_path.push_back(path[0]);

    int curr = 0;

    for (int i = 2; i < path.size(); i++) {
        if (!walkable_bresenhams_line(path[curr], path[i])) {
            pruned_path.push_back(path[i - 1]);
            curr = i - 1;
        }
    }

    pruned_path.push_back(path.back());
    return pruned_path;
}

std::vector<vec2> PathfindingSystem::path_finding(const vec2 &start_world, const vec2 &goal_world) {
    ivec2 start = world_to_tile(start_world);
    ivec2 goal = world_to_tile(goal_world);

    if (!registry.tilemaps.has(current_map)) {
        current_map = tms->get_current_map();
    }
    TileMap &map = registry.tilemaps.get(current_map);

    if (is_out_of_bounds(start) || is_out_of_bounds(goal)) {
        return {};
    }

    const int width = map.width;
    const int height = map.height;
    const int total = width * height;

    std::vector<int> g_cost(total, INT_MAX);
    std::vector<int> parent(total, -1);
    std::vector<bool> closed(total, false);

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    int start_idx = get_map_index(width, start.x, start.y);
    g_cost[start_idx] = 0;

    open.push({start.x, start.y, 0, h_cost(start, goal)});

    // algorithm loop
    while (!open.empty()) {
        Node current = open.top();
        open.pop();

        int idx = get_map_index(width, current.x, current.y);

        if (closed[idx]) {
            continue;
        }

        closed[idx] = true;

        if (current.x == goal.x && current.y == goal.y) {
            break;
        }

        for (const auto &dir : four_directions) {
            int neighbour_x = current.x + dir.x;
            int neighbour_y = current.y + dir.y;

            if (!is_walkable(neighbour_x, neighbour_y)) {
                continue;
            }

            int neighbour_id = get_map_index(width, neighbour_x, neighbour_y);

            if (closed[neighbour_id]) {
                continue;
            }

            int tentative_g = g_cost[idx] + 1;

            if (tentative_g < g_cost[neighbour_id]) {
                g_cost[neighbour_id] = tentative_g;
                parent[neighbour_id] = idx;

                int f = tentative_g + h_cost({neighbour_x, neighbour_y}, goal);

                open.push({neighbour_x, neighbour_y, tentative_g, f});
            }
        }
    }

    int goal_idx = get_map_index(width, goal.x, goal.y);
    if (parent[goal_idx] == -1 && !(start == goal)) {
        return {};
    }

    // Reconstruct and convert tile path
    std::vector<ivec2> tile_path;
    int p = goal_idx;

    while (p != -1) {
        int tx = p % width;
        int ty = p / width;
        tile_path.push_back({tx, ty});
        p = parent[p];
    }

    tile_path = prune_path(tile_path);
    std::reverse(tile_path.begin(), tile_path.end());

    std::vector<vec2> world_path;
    world_path.reserve(tile_path.size());

    // add small buffer to prevent wall sticking
    for (const auto &t : tile_path) {
        vec2 world_coord = tile_center_to_world(t);
        float buffer = TILE_SIZE_PX * 0.25f;

        vec2 offset = vec2(0.f);

        if (!is_walkable(t.x + 1, t.y + 1)) {
            offset.x -= buffer;
            offset.y -= buffer;
        }
        if (!is_walkable(t.x + 1, t.y - 1)) {
            offset.x -= buffer;
            offset.y += buffer;
        }
        if (!is_walkable(t.x - 1, t.y + 1)) {
            offset.x += buffer;
            offset.y -= buffer;
        }
        if (!is_walkable(t.x - 1, t.y - 1)) {
            offset.x += buffer;
            offset.y += buffer;
        }
        if (!is_walkable(t.x + 1, t.y)) {
            offset.x -= buffer;
        }
        if (!is_walkable(t.x - 1, t.y)) {
            offset.x += buffer;
        }
        if (!is_walkable(t.x, t.y + 1)) {
            offset.y -= buffer;
        }
        if (!is_walkable(t.x, t.y - 1)) {
            offset.y += buffer;
        }

        world_path.push_back(world_coord + offset);
    }

    return world_path;
}
#pragma endregion Pathing

#pragma region Bezier
vec2 PathfindingSystem::evaluate_cubic_bezier(const BezierPath &path, float t) {
    float u = 1.f - t;

    return u * u * u * path.p0 + 3.f * u * u * t * path.p1 + 3.f * u * t * t * path.p2 + t * t * t * path.p3;
}

void PathfindingSystem::update_leader(Entity e, Motion &motion, float elapsed_ms) {
    if (!registry.tilemaps.has(current_map)) {
        current_map = tms->get_current_map();
    }
    TileMap &map = registry.tilemaps.get(current_map);
    if (map.patrol_loop.empty()) {
        return;
    }

    Bezier &b = registry.beziers.get(e);

    // advance along the curve
    b.t += b.speed * (elapsed_ms / 1000.f);

    if (b.t >= 1.f) {
        b.t = 0.f;
        b.current_segment = (b.current_segment + 1) % map.patrol_loop.size();
    }

    BezierPath &seg = map.patrol_loop[b.current_segment];

    // evaluate spline
    float lookahead = b.t + 0.01f;
    BezierPath *look_seg = &seg;

    if (lookahead > 1.f) {
        lookahead -= 1.f;
        look_seg = &map.patrol_loop[(b.current_segment + 1) % map.patrol_loop.size()];
    }

    vec2 p1 = evaluate_cubic_bezier(seg, b.t);
    vec2 target = evaluate_cubic_bezier(seg, b.t);
    vec2 p2 = evaluate_cubic_bezier(*look_seg, lookahead);
    vec2 dir = normalize(p2 - target);

    motion.position = target;
    motion.angle = atan2(dir.y, dir.x);
    b.velocity = (p2 - p1) / 0.01f;
}
#pragma endregion Bezier

#pragma region Boids
// modeled after Ben Eater's Boids Demo
// https://github.com/beneater/boids

vec2 PathfindingSystem::compute_alignment(Entity e, std::vector<Entity> &flock) {
    Motion &motion = registry.motions.get(e);

    vec2 avg(0.f);
    int count = 0;

    for (Entity other : flock) {
        Motion &om = registry.motions.get(other);

        avg += om.velocity;
        count++;
    }

    if (count == 0)
        return vec2(0);

    avg /= (float)count;

    return (avg - motion.velocity) * 0.05f;
}

vec2 PathfindingSystem::compute_cohesion(Entity e, std::vector<Entity> &flock) {
    Boid &self = registry.boids.get(e);
    Motion &motion = registry.motions.get(e);

    vec2 center(0.f);
    int count = 0;

    for (Entity other : flock) {
        Motion &om = registry.motions.get(other);

        if (length(om.position - motion.position) < self.perception_radius) {
            center += om.position;
            count++;
        }
    }

    if (count == 0) {
        return vec2(0);
    }

    center = center / (float)count;

    return (center - motion.position) * 0.005f;
}

vec2 PathfindingSystem::compute_separation(Entity e, std::vector<Entity> &flock) {
    Motion &motion = registry.motions.get(e);

    float separation_radius = TILE_SIZE_PX/2;
    float separation_strength = 80.f;

    vec2 force = vec2(0.f);
    int count = 0;

    for (Entity other : flock) {
        Motion &om = registry.motions.get(other);

        vec2 diff = motion.position - om.position;
        float dist = length(diff);

        if (dist > 0.f && dist < separation_radius) {
            vec2 dir = normalize(diff);

            // stronger push when very close
            float weight = (separation_radius - dist) / separation_radius;

            force += dir * weight;
            count++;
        }
    }

    if (count > 0)
        force /= (float)count;

    return force * separation_strength;
}

vec2 PathfindingSystem::compute_follow_force(Entity follower, Entity leader) {
    Motion &fm = registry.motions.get(follower);
    Motion &lm = registry.motions.get(leader);

    vec2 leader_dir = normalize(registry.beziers.get(leader).velocity);

    vec2 target = lm.position - leader_dir * (TILE_SIZE_PX * 1.5f);

    return (target - fm.position) * 0.01f;
}

vec2 PathfindingSystem::avoid_walls(Entity e) {
    Motion &motion = registry.motions.get(e);
    TileMap &map = registry.tilemaps.get(current_map);

    vec2 force = vec2(0.f);

    float avoid_radius = TILE_SIZE_PX * 1.5f;
    float avoid_factor = 60.f;

    ivec2 tile = world_to_tile(motion.position);

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {

            ivec2 check_tile = tile + ivec2(dx, dy);

            if (check_tile.x < 0 || check_tile.y < 0 || check_tile.x >= map.width || check_tile.y >= map.height) {
                continue;
            }

            int idx = check_tile.y * map.width + check_tile.x;
            if (map.tile_ids[idx] == 0) {
                continue;
            }

            vec2 wall_center = tile_center_to_world(check_tile);
            vec2 diff = motion.position - wall_center;

            float dist = length(diff);

            if (dist < avoid_radius && dist > 0.f) {
                vec2 push = normalize(diff);
                float strength = (avoid_radius - dist) / avoid_radius;

                force += push * strength * avoid_factor;
            }
        }
    }

    return force;
}

vec2 PathfindingSystem::compute_boid_force(Entity e, std::vector<Entity> &flock, Entity leader) {
    Boid &boid = registry.boids.get(e);
    vec2 align = compute_alignment(e, flock) * boid.alignment_weight;
    vec2 cohesion = compute_cohesion(e, flock) * boid.cohesian_weight;
    vec2 separation = compute_separation(e, flock) * boid.separation_weight;

    return align + cohesion + separation;
}

vec2 limit_speed(vec2 velocity) {
    float speed = length(velocity);
    float max_speed = 150.f;

    if (speed > max_speed) {
        return (velocity / speed) * max_speed;
    }

    return velocity;
}

void PathfindingSystem::update_follower(Entity follower, Entity leader, float elapsed_ms) {
    Motion &fm = registry.motions.get(follower);
    Boid &boid = registry.boids.get(follower);
    Motion &lm = registry.motions.get(leader);

    std::vector<Entity> flock;
    for (Entity bird : registry.boids.entities) {
        if (bird == follower) {
            continue;
        }
        if (registry.boids.get(bird).leader != leader) {
            continue;
        }
        flock.push_back(bird);
    }

    boid.velocity += compute_boid_force(follower, flock, leader) + avoid_walls(follower) + compute_follow_force(follower, leader);
    boid.velocity = limit_speed(boid.velocity);

    fm.angle = atan2(boid.velocity.y, boid.velocity.x);
    fm.velocity = vec2(length(boid.velocity), 0.f);
}
#pragma endregion Boids
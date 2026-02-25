#include "pathfinding_system.hpp"
#include <queue>
#include "../tinyECS/registry.hpp"

const ivec2 four_directions[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

struct Node {
    int x, y;
    int g;
    int f;

    bool operator>(const Node &other) const { return f > other.f; }
};

int get_map_index(int width, int x, int y) { 
    return x + (y * width); 
}

bool is_out_of_bounds(int width, int height, int x, int y) { return x < 0 || y < 0 || x >= width || y >= height; }

static bool is_walkable(const TileMap &map, int x, int y) {
    if (is_out_of_bounds((int)map.width, (int)map.height, x, y)) {
        return false;
    }
    int tile_id = map.tile_ids[x + y * map.width];

    bool is_empty = (tile_id == 1);
    if (!is_empty) {
        return false;
    }

    return true;
}

int h_cost(const ivec2 &a, const ivec2 &b) { 
    int dx = abs(a.x - b.x); 
    int dy = abs(a.y - b.y);

    int min_d = std::min(dx, dy);
    int max_d = std::max(dx, dy);

    return (14 * min_d) + (10 * (max_d - min_d));
}

bool walkable_bresenhams_line(ivec2 a, ivec2 b, const TileMap &map) { 
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
        int prev_x = x1;
        int prev_y = y1;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }

        // modify Bresenham's to check both tiles in a diagonal
        if (x1 != prev_x && y1 != prev_y) {
            if (!is_walkable(map, prev_x, y1) || !is_walkable(map, x1, prev_y)) {
                return false;
            }
        }
    }

    return true;
}

std::vector<ivec2> prune_path(std::vector<ivec2> &path, const TileMap &map) { 
    if (path.size() <= 2) {
        return path;
    }

    std::vector<ivec2> pruned_path;
    pruned_path.push_back(path[0]);

    int curr = 0;

    for (int i = 2; i < path.size(); i++) {
        if (!walkable_bresenhams_line(path[curr], path[i], map)) {
            pruned_path.push_back(path[i - 1]);
            curr = i - 1;
        }
    }

    pruned_path.push_back(path.back());
    return pruned_path;
}

std::vector<vec2> path_finding_bfs(const vec2 &start_world, const vec2 &goal_world) {
    const TileMap &map = registry.tilemaps.components[0];

    ivec2 start = world_to_tile(start_world);
    ivec2 goal = world_to_tile(goal_world);

    if (is_out_of_bounds((int)map.width, (int)map.height, start.x, start.y) ||
        is_out_of_bounds((int)map.width, (int)map.height, goal.x, goal.y)) {
        return {};
    }       
    
    const int width = map.width;
    const int height = map.height;
    const int total = width * height;

    std::vector<bool> visited(total, false);
    std::vector<int> parent(total, -1);

    std::queue<ivec2> queue;

    queue.push(start);
    visited[get_map_index(width, start.x, start.y)] = true;

    // algorithm loop
    while (!queue.empty()) {
        ivec2 current = queue.front();
        queue.pop();

        if (current.x == goal.x && current.y == goal.y)
            break;

        for (const auto &dir : four_directions) {
            int neighbour_x = current.x + dir.x;
            int neighbour_y = current.y + dir.y;

            if (!is_walkable(map, neighbour_x, neighbour_y) && !(neighbour_x == goal.x && neighbour_y == goal.y))
                continue;

            int idx = get_map_index(width, neighbour_x, neighbour_y);

            if (visited[idx])
                continue;

            visited[idx] = true;
            parent[idx] = get_map_index(width, current.x, current.y);
            queue.push({neighbour_x, neighbour_y});
        }
    }

    // check if goal visited
    int goal_index = get_map_index(width, goal.x, goal.y);
    if (!visited[goal_index]) {
        return {};
    }

    // reconstruct and convert tile path
    std::vector<ivec2> tile_path;
    int p = goal_index;

    while (p != -1) {
        int tx = p % width;
        int ty = p / width;
        tile_path.push_back({tx, ty});
        p = parent[p];
    }

    tile_path = prune_path(tile_path, map);
    std::reverse(tile_path.begin(), tile_path.end());

    std::vector<vec2> world_path;
    world_path.reserve(tile_path.size());

    for (const auto &tile : tile_path) {
        vec2 world_coord = tile_center_to_world(tile);
        world_path.push_back(world_coord);
    }
        

    return world_path;
}

std::vector<vec2> path_finding_a(const vec2& start_world, const vec2& goal_world) {
    const TileMap &map = registry.tilemaps.components[0];

    ivec2 start = world_to_tile(start_world);
    ivec2 goal = world_to_tile(goal_world);

    if (is_out_of_bounds((int)map.width, (int)map.height, start.x, start.y) ||
        is_out_of_bounds((int)map.width, (int)map.height, goal.x, goal.y)) {
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

            if (!is_walkable(map, neighbour_x, neighbour_y)) {
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

    tile_path = prune_path(tile_path, map);
    std::reverse(tile_path.begin(), tile_path.end());

    std::vector<vec2> world_path;
    world_path.reserve(tile_path.size());

    for (const auto &t : tile_path) {
        world_path.push_back(tile_center_to_world(t));
    }
        

    return world_path;
}
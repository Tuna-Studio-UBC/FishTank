#include "pathfinding_system.hpp"
#include <algorithm>
#include <array>
#include <queue>
#include "../tinyECS/registry.hpp"
#include <spdlog/spdlog.h>

const Int2 four_directions[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
const Int2 eight_directions[8] = {
    {1, 0}, {-1, 0}, {0, 1},  {0, -1}, // straight
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1} // diagonal
};

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
        //spdlog::debug("neighbour is out of bounds");
        return false;
    }
    int tile_id = map.tile_ids[x + y * map.width];

    bool is_empty = (tile_id == 1);
    if (!is_empty) {
        //spdlog::debug("neighbour is not empty");
        return false;
    }

    //spdlog::debug("found walkable");
    return true;
}

int h_cost(const Int2 &a, const Int2 &b) { 
    int dx = abs(a.x - b.x); 
    int dy = abs(a.y - b.y);

    int min_d = std::min(dx, dy);
    int max_d = std::max(dx, dy);

    return (14 * min_d) + (10 * (max_d - min_d));
}

std::vector<vec2> path_finding_bfs(const vec2 &start_world, const vec2 &goal_world) {
    const TileMap &map = registry.tilemaps.components[0];

    Int2 start = world_to_tile(start_world);
    Int2 goal = world_to_tile(goal_world);

    if (is_out_of_bounds((int)map.width, (int)map.height, start.x, start.y) ||
        is_out_of_bounds((int)map.width, (int)map.height, goal.x, goal.y)) {
        return {};
    }       
    
    const int width = map.width;
    const int height = map.height;
    const int total = width * height;

    std::vector<bool> visited(total, false);
    std::vector<int> parent(total, -1);

    std::queue<Int2> queue;

    queue.push(start);
    visited[get_map_index(width, start.x, start.y)] = true;

    // algorithm loop
    while (!queue.empty()) {
        Int2 current = queue.front();
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
        //spdlog::debug("goal cannot be reached");
        return {};
    }

    // reconstruct and convert tile path
    std::vector<Int2> tile_path;
    int p = goal_index;

    while (p != -1) {
        int tx = p % width;
        int ty = p / width;
        tile_path.push_back({tx, ty});
        p = parent[p];
    }

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

    Int2 start = world_to_tile(start_world);
    Int2 goal = world_to_tile(goal_world);

    if (is_out_of_bounds((int)map.width, (int)map.height, start.x, start.y) ||
        is_out_of_bounds((int)map.width, (int)map.height, goal.x, goal.y)) {
        spdlog::debug("goal or start out of bounds");
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
                spdlog::debug("neighbour isn't walkable");
                continue;
            }

            int neighbour_id = get_map_index(width, neighbour_x, neighbour_y);

            if (closed[neighbour_id]) {
                spdlog::debug("neighbour is closed");
                continue;
            }

            int tentative_g = g_cost[idx] + 1;

            if (tentative_g < g_cost[neighbour_id]) {
                g_cost[neighbour_id] = tentative_g;
                parent[neighbour_id] = idx;

                int f = tentative_g + h_cost({neighbour_x, neighbour_y}, goal);

                open.push({neighbour_x, neighbour_y, tentative_g, f});
                spdlog::debug("pushing ({},{}) to open", neighbour_x, neighbour_y);
            }
        }
    }

    int goal_idx = get_map_index(width, goal.x, goal.y);
    if (parent[goal_idx] == -1 && !(start == goal)) {
        return {};
    }

    // Reconstruct and convert tile path
    std::vector<Int2> tile_path;
    int p = goal_idx;

    while (p != -1) {
        int tx = p % width;
        int ty = p / width;
        tile_path.push_back({tx, ty});
        p = parent[p];
    }

    std::reverse(tile_path.begin(), tile_path.end());

    std::vector<vec2> world_path;
    world_path.reserve(tile_path.size());

    for (const auto &t : tile_path) {
        spdlog::debug("pushing tile ({},{}) to path", t.x, t.y);
        world_path.push_back(tile_center_to_world(t));
    }
        

    return world_path;
}
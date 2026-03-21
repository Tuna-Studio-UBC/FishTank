#include "tilemap_system.hpp"

#include <spdlog/spdlog.h>

#include "common.hpp"
#include "systems/world/world_init.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// #define DEBUG_INIT_TILES

#ifdef DEBUG_INIT_TILES
void print_tilemap(TileMap &tilemap) {
    std::cout << "TILEMAP: \n[\n";
    for (int y = 0; y < tilemap.height; y++) {
        std::cout << "[";
        for (int x = 0; x < tilemap.width; x++) {
            std::cout << tilemap.tile_ids[x + tilemap.width * y] << ", ";
        }
        std::cout << "]\n";
    }
    std::cout << "]\n";
}

void print_tileset(TileSet &tileset) {
    std::cout << "TILESET: \n[\n";
    for (int y = 0; y < tileset.dimensions.y; y++) {
        std::cout << "[";
        for (int x = 0; x < tileset.dimensions.x; x++) {
            std::cout << (int)tileset.collisions[x + tileset.dimensions.x * y] << ", ";
        }
        std::cout << "]\n";
    }
    std::cout << "]\n";
}
#endif

// void generate_collision(TileMap &tilemap, TileSet &tileset) {
//     auto &tileset_collision_info = tileset.collisions;
//     for (int y = 1; y < tilemap.height - 1; y++) {
//         for (int x = 1; x < tilemap.width - 1; x++) {
//             if (tileset_collision_info[tilemap.tile_ids[GET_2D_POS(x, y, tilemap.width)] - 1]) {
//                 createWall(vec2(x * TILE_SIZE_PX, y * TILE_SIZE_PX));
//             }
//         }
//     }
// }

TileMapSystem::TileMapSystem() { rng = std::default_random_engine(std::random_device()()); }
TileMapSystem::~TileMapSystem() { clear_map(); }

Entity TileMapSystem::init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                                 int layer) {
    Entity entity = Entity(map_file_name);

    TileMap tilemap = parse_tilemap(get_path(FilePath::MAPS, map_file_name));
    TileSet tileset = parse_tileset(get_path(FilePath::TILESETS, tileset_file_name));

    registry.tilemaps.insert(entity, tilemap);
    registry.tilesets.insert(entity, tileset);

    registry.renderRequests.insert(
        entity, {textures_id, EFFECT_ASSET_ID::TILEMAP, GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::BACKGROUND + layer});

#ifdef DEBUG_INIT_TILES
    print_tilemap(tilemap);
    print_tileset(tileset);
#endif

    return entity;
}

void TileMapSystem::init_tile_walls(TileMap *tilemap) {
    int walls_created = 0;
    for (unsigned int y = 0; y < tilemap->height; y++) {
        for (unsigned int x = 0; x < tilemap->width; x++) {
            int tile_id = tilemap->tile_ids[x + y * tilemap->width];

            // skip empty/floor tiles, skip outer border tiles
            bool is_border = (y == 0 || y == tilemap->height - 1 || x == 0 || x == tilemap->width - 1);
            bool is_empty = is_empty_tile(tile_id);

            if (!is_empty && !is_border) {
                vec2 position = vec2(x * TILE_SIZE_PX + TILE_SIZE_PX / 2.f, y * TILE_SIZE_PX + TILE_SIZE_PX / 2.f);
                vec2 scale = vec2(TILE_SIZE_PX, TILE_SIZE_PX);
                createWall(position, scale);
                walls_created++;
            }
        }
    }
    // edge walls
    float arena_width = tilemap->width * TILE_SIZE_PX;
    float arena_height = tilemap->height * TILE_SIZE_PX;
    float wall_thickness = TILE_SIZE_PX;
    // top
    createWall(vec2(arena_width / 2.f, wall_thickness / 2.f), vec2(arena_width, wall_thickness));
    // bottom
    createWall(vec2(arena_width / 2.f, arena_height - wall_thickness / 2.f), vec2(arena_width, wall_thickness));
    // left
    createWall(vec2(wall_thickness / 2.f, arena_height / 2.f), vec2(wall_thickness, arena_height));
    // right
    createWall(vec2(arena_width - wall_thickness / 2.f, arena_height / 2.f), vec2(wall_thickness, arena_height));
}

// Helpers to shrink radius
enum FailAxis { NOFAIL, FAIL_X, FAIL_Y };

FailAxis ellipse_fail_axis(vec2 center, float rx, float ry, TileMap *map) {
    int samples = (int)((rx + ry) * 0.1f);
    samples = std::max(samples, 64);
    const float entity_radius = TILE_SIZE_PX * 0.4f;

    for (int i = 0; i < samples; i++) {
        float t = (2.f * M_PI * i) / samples;

        vec2 dir = vec2(cos(t), sin(t));
        vec2 p = center + vec2(rx * dir.x, ry * dir.y);

        p += normalize(dir) * entity_radius;

        ivec2 tile = world_to_tile(p);

        if (tile.x <= 0 || tile.y <= 0 || tile.x >= (int)map->width - 1 || tile.y >= (int)map->height - 1) {
            if (abs(dir.x) > abs(dir.y)) {
                return FAIL_X;
            } else {
                return FAIL_Y;
            }
        }

        int idx = get_map_index(map->width, tile.x,  tile.y);
        if (!is_empty_tile(map->tile_ids[idx])) {
            spdlog::debug("wall");
            if (abs(dir.x) > abs(dir.y)) {
                return FAIL_X;
            } else {
                return FAIL_Y;
            }
        }
    }

    return NOFAIL;
}

void TileMapSystem::generate_loop(TileMap *map) {
    std::vector<BezierPath> loop;

    const float entity_radius = TILE_SIZE_PX;
    const float margin = TILE_SIZE_PX * 0.3f;
    const float step = TILE_SIZE_PX * 0.5f;
    vec2 center = vec2((map->width * TILE_SIZE_PX) * 0.5f, (map->height * TILE_SIZE_PX) * 0.5f);

    // start from biggest radius, shrink down till valid
    float max_rx = (map->width * TILE_SIZE_PX) * 0.5f;
    float max_ry = (map->height * TILE_SIZE_PX) * 0.5f;

    float rx = max_rx;
    float ry = max_ry;

    bool found = false;
    while (rx > TILE_SIZE_PX && ry > TILE_SIZE_PX) {
        FailAxis fail = ellipse_fail_axis(center, rx, ry, map);

        if (fail == NONE) {
            found = true;
            break;
        }

        if (fail == FAIL_X) {
            rx -= step;
        } else if (fail == FAIL_Y) {
            ry -= step;
        }
    }

    if (!found) {
        spdlog::warn("No valid centered loop found");
        map->patrol_loop.clear();
        return;
    }

    rx -= margin;
    ry -= margin;

    // cubic bezier approximation c
    const float c = (4/3)*(sqrt(2)-1);

    vec2 top = center + vec2(0, -ry);
    vec2 right = center + vec2(rx, 0);
    vec2 bottom = center + vec2(0, ry);
    vec2 left = center + vec2(-rx, 0);

    loop.push_back({top, top + vec2(c * rx, 0), right + vec2(0, -c * ry), right});

    loop.push_back({right, right + vec2(0, c * ry), bottom + vec2(c * rx, 0), bottom});

    loop.push_back({bottom, bottom + vec2(-c * rx, 0), left + vec2(0, c * ry), left});

    loop.push_back({left, left + vec2(0, -c * ry), top + vec2(-c * rx, 0), top});

    // uncomment below to check generated loop
    // spdlog::debug("Generated centered loop at ({}, {}), rx={}, ry={}", center.x, center.y, rx, ry);

    map->patrol_loop = loop;
}

void TileMapSystem::init_spawn_points() {
    spawn_tiles.clear();
    TileMap &tilemap = registry.tilemaps.get(current_map);
    TileSet &tileset = registry.tilesets.get(current_map);

    for (unsigned int y = 0; y < tilemap.height; y++) {
        for (unsigned int x = 0; x < tilemap.width; x++) {
            int spawn_tile_id = tilemap.spawn_tile_ids[x + y * tilemap.width];
            spawn_tiles.push_back(std::pair<SPAWN_TARGET, vec2>((SPAWN_TARGET)spawn_tile_id, {x, y}));
        }
    }
}

vec2 TileMapSystem::get_random_position(SPAWN_TARGET spawn_target) {
    std::vector<int> spawnable_tiles;
    for (int i = 0; i < spawn_tiles.size(); i++) {
        auto spawn_tile = spawn_tiles[i];
        if (spawn_tile.first != SpawnTarget::NONE && spawn_tile.first & spawn_target ||
            spawn_tile.first == SpawnTarget::ANY) {
            spawnable_tiles.push_back(i);
        }
    }

    // No spawnable tiles, return invalid position
    if (spawnable_tiles.empty())
        return vec2(-1, -1);

    int spawnable_idx = (uniform_dist(rng) * spawnable_tiles.size()) - 1;
    int tile_idx = spawnable_tiles[spawnable_idx];
    vec2 tile = spawn_tiles[tile_idx].second;
    spawn_tiles[tile_idx] = spawn_tiles.back();
    spawn_tiles.pop_back();

    spdlog::debug("Spawning {} at tile: {} {}", spawn_target, tile.x, tile.y);

    return vec2(tile.x * TILE_WIDTH_PX + (TILE_WIDTH_PX / 2), tile.y * TILE_HEIGHT_PX + (TILE_HEIGHT_PX / 2));
}

void TileMapSystem::clear_map() {
    for (auto entity : registry.tilemaps.entities)
        registry.remove_all_components_of(entity);
    for (auto entity : registry.tilesets.entities)
        registry.remove_all_components_of(entity);
    for (auto entity : registry.walls.entities)
        registry.remove_all_components_of(entity);
}

Entity TileMapSystem::load_map(MapType map_type) {
    clear_map();
    current_map = LOAD_MAP_FN[map_type]();
    spdlog::debug("Loaded tile map: {}", current_map.name());
    return current_map;
}

#pragma region Maps
// =================================================================================
// Maps
// =================================================================================

Entity TileMapSystem::load_empty_map() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity map = init_tiles("empty_map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    return map;
}

Entity TileMapSystem::load_test_map() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity map = init_tiles("map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    generate_loop(&registry.tilemaps.get(map));
    return map;
}

Entity TileMapSystem::load_test_map2() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity map = init_tiles("map2.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    generate_loop(&registry.tilemaps.get(map));
    return map;
}

Entity TileMapSystem::load_tutorial_map() {
    init_tiles("tutorial_background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, -1);
    Entity map = init_tiles("tutorial_map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    return map;
}

#pragma endregion Maps

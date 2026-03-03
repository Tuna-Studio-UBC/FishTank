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
    for (auto entity : registry.wall.entities)
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

Entity TileMapSystem::load_test_map() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity map = init_tiles("map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    return map;
}

Entity TileMapSystem::load_test_map2() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity map = init_tiles("map2.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    return map;
}

Entity TileMapSystem::load_tutorial_map() {
    init_tiles("tutorial_background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, -1);
    Entity map = init_tiles("tutorial_map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2, 1);
    init_tile_walls(&registry.tilemaps.get(map));
    return map;
}

#pragma endregion Maps

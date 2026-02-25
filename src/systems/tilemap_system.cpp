#include "tilemap_system.hpp"

#include "../common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "world_init.hpp"

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

void generate_collision(TileMap &tilemap, TileSet &tileset) {
    auto &tileset_collision_info = tileset.collisions;
    for (int y = 1; y < tilemap.height - 1; y++) {
        for (int x = 1; x < tilemap.width - 1; x++) {
            if (tileset_collision_info[tilemap.tile_ids[GET_2D_POS(x, y, tilemap.width)] - 1]) {
                createWall(vec2(x * TILE_SIZE_PX, y * TILE_SIZE_PX));
            }
        }
    }
}

Entity TileMapSystem::init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                                 int layer) {
    Entity entity = Entity("Tiles");

    TileMap tilemap = parse_tilemap(get_path(FilePath::MAPS, map_file_name));
    TileSet tileset = parse_tileset(get_path(FilePath::TILESETS, tileset_file_name));

    registry.tilemaps.insert(entity, tilemap);
    registry.tilesets.insert(entity, tileset);

    registry.renderRequests.insert(entity, {textures_id, EFFECT_ASSET_ID::TILEMAP, GEOMETRY_BUFFER_ID::SPRITE, layer});

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
}

#pragma region Maps
// =================================================================================
// Maps
// =================================================================================

Entity TileMapSystem::load_test_map() {
    init_tiles("background.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, -1);
    Entity current_map = init_tiles("map.json", "tileset.json", TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET, 1);
    init_tile_walls(&registry.tilemaps.get(current_map));
    return current_map;
}

#pragma endregion Maps

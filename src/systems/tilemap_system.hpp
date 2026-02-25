#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "tinyECS/components.hpp"

const enum class MapType {
    TEST_MAP = 0,
};

class TileMapSystem {
  private:
    static Entity load_test_map();

    static Entity init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                             int layer = 0);
    static void init_tile_walls(TileMap *tilemap);

  public:
    inline static std::unordered_map<MapType, std::function<Entity()>> TileMapSystem::LOAD_MAP_FN = {
        {MapType::TEST_MAP, load_test_map}};

    static Entity load_map(MapType map_type = MapType::TEST_MAP) { return LOAD_MAP_FN[map_type](); };
};

#pragma once

#include "tinyECS/components.hpp"

class TileMapSystem {
  public:
    static Entity load_test_map();

  private:
    static Entity init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                             int layer = 0);
    static void init_tile_walls(TileMap *tilemap);
};

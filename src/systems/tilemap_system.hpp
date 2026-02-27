#pragma once

#include <functional>
#include <random>
#include <string>
#include <unordered_map>

#include "tinyECS/components.hpp"

const enum class MapType {
    TEST_MAP = 0,
    TEST_MAP2 = TEST_MAP + 1,
    TUTORIAL_MAP,
};

class TileMapSystem {
  private:
    Entity current_map;
    std::vector<vec2> free_tiles;

    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;

    static Entity init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                             int layer = 0);
    static void init_tile_walls(TileMap *tilemap);

    static Entity load_test_map();
    static Entity load_test_map2();
    static Entity load_tutorial_map();


  public:
    TileMapSystem() {};
    TileMapSystem(std::default_random_engine rng) { this->rng = rng; }
    ~TileMapSystem();

    void init_free_tiles();
    vec2 get_random_position();
    Entity get_current_map() const { return current_map; }

    Entity load_map(MapType map_type = MapType::TEST_MAP);

    std::unordered_map<MapType, std::function<Entity()>> LOAD_MAP_FN = {};

    void init(std::default_random_engine rng) {
        this->rng = rng;

        // Define loadable maps here
        LOAD_MAP_FN.emplace(MapType::TEST_MAP, TileMapSystem::load_test_map);
        LOAD_MAP_FN.emplace(MapType::TEST_MAP2, TileMapSystem::load_test_map2);
        LOAD_MAP_FN.emplace(MapType::TUTORIAL_MAP, TileMapSystem::load_tutorial_map);
    }
};

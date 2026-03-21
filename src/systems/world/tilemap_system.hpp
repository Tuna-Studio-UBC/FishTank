#pragma once

#include <functional>
#include <random>
#include <string>
#include <unordered_map>

#include "configs/spawn_target_config.hpp"
#include "tinyECS/components.hpp"

const enum class MapType {
    TUTORIAL_MAP,
    EMPTY_MAP,
    TEST_MAP,
    TEST_MAP2,
};

class TileMapSystem {
  private:
    Entity current_map;
    std::vector<std::pair<SPAWN_TARGET, vec2>> spawn_tiles;

    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;

    static Entity init_tiles(std::string map_file_name, std::string tileset_file_name, TEXTURE_ASSET_ID textures_id,
                             int layer = 0);
    static void init_tile_walls(TileMap *tilemap);

    static Entity load_empty_map();
    static Entity load_test_map();
    static Entity load_test_map2();
    static Entity load_tutorial_map();

    static std::vector<BezierPath> generate_loop(const TileMap &map);

  public:
    TileMapSystem();
    ~TileMapSystem();

    void init_spawn_points();
    vec2 get_random_position(SPAWN_TARGET spawn_target);
    Entity get_current_map() const { return current_map; }

    void clear_map();
    Entity load_map(MapType map_type = MapType::TEST_MAP);

    std::unordered_map<MapType, std::function<Entity()>> LOAD_MAP_FN = {};

    void init() {
        this->rng = std::default_random_engine(std::random_device()());

        // Define loadable maps here
        LOAD_MAP_FN.emplace(MapType::EMPTY_MAP, TileMapSystem::load_empty_map);
        LOAD_MAP_FN.emplace(MapType::TEST_MAP, TileMapSystem::load_test_map);
        LOAD_MAP_FN.emplace(MapType::TEST_MAP2, TileMapSystem::load_test_map2);
        LOAD_MAP_FN.emplace(MapType::TUTORIAL_MAP, TileMapSystem::load_tutorial_map);
    }
};

#include "enemy_config.hpp"
#include "hazard_config.hpp"
#include "power_up_config.hpp"
#include "systems/world/tilemap_system.hpp"

#include <vector>

struct Wave {
    std::vector<ENEMY_TYPE> enemies;
};

struct Level {
    int last_wave;
    MapType map_type;
    std::vector<HAZARD_TYPE> hazards;
    std::vector<POWER_UP_TYPE> powerups;
    std::vector<float> bells;
    std::vector<float> bubblers; // TODO: Change type once bubblers are further developed
    std::vector<int> swarms;     // Secret for M3, leave it empty for now
    bool upgrade_screen_before = false;
};

const std::vector<Wave> WAVE_CONFIGS = {
    // Level 1
    {{ENEMY_TYPE::GUARD}},
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN}},

    // Level 2
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}}};

const std::vector<Level> LEVEL_CONFIGS = {
    // Level 1
    {2, MapType::TEST_MAP, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {}, {0.f, 0.f}, {}, true},

    // Level 2
    {5, MapType::TEST_MAP2, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {}, {0.f, 0.f}, {}}};

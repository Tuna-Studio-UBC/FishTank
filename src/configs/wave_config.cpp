#include "enemy_config.hpp"
#include "hazard_config.hpp"
#include "power_up_config.hpp"
#include "systems/world/tilemap_system.hpp"

#include <vector>

struct Wave {
    std::vector<ENEMY_TYPE> enemies;
    std::vector<BOSS_TYPE> bosses;
    bool final_wave = false;
};

struct Level {
    // int last_wave;
    MapType map_type;
    std::vector<HAZARD_TYPE> hazards;
    std::vector<POWER_UP_TYPE> powerups;
    std::vector<float> bells;
    std::vector<float> bubblers; // TODO: Change type once bubblers are further developed
    bool upgrade_screen_before = false;
    float spawn_threshold = 0.5f;
    float time_threshold = 15000.f;
};

const std::vector<Wave> WAVE_CONFIGS = {
    // Level 1
    {{ENEMY_TYPE::GUARD}},
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN}},
    {{ENEMY_TYPE::MOTHER, ENEMY_TYPE::GUARD, ENEMY_TYPE::GUARD}, {}, true},

    // Level 2
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}, {}, true},

    // Level 3: Boss
    {{ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}, {BOSS_TYPE::SHARKY}, true},

    // Level 4
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}},
    {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN, ENEMY_TYPE::URCHIN, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}, {}, true},

    // Level 5: Boss
    {{ENEMY_TYPE::MOTHER, ENEMY_TYPE::GUARD, ENEMY_TYPE::GUARD}, {BOSS_TYPE::KING_CRABBY}, true}
};

const std::vector<Level> LEVEL_CONFIGS = {
    // Level 1
    {MapType::TEST_MAP, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {}, {0.f, 0.f}, true},

    // Level 2
    {MapType::TEST_MAP2,
    {HAZARD_TYPE::MINE, HAZARD_TYPE::ANEMONE},
    {POWER_UP_TYPE::HEAL, POWER_UP_TYPE::SPEED_MULTIPLIER},
    {},
    {0.f, 0.f}},
    
    // Level 3
    {MapType::EMPTY_MAP,
     {HAZARD_TYPE::MINE, HAZARD_TYPE::MINE},
     {POWER_UP_TYPE::HEAL, POWER_UP_TYPE::HEAL, POWER_UP_TYPE::HEAL, POWER_UP_TYPE::SPEED_MULTIPLIER},
     {},
     {0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
     false},

    // Level 4
    {MapType::TEST_MAP3, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {}, {0.f, 0.f}, true},
    // Level 5
    {MapType::TEST_MAP,
     {HAZARD_TYPE::ANEMONE, HAZARD_TYPE::ANEMONE},
     {POWER_UP_TYPE::HEAL, POWER_UP_TYPE::HEAL, POWER_UP_TYPE::DAMAGE_MULTIPLIER},
     {},
     {0.f, 0.f, 0.f},
     true}
};

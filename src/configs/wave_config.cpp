#include "enemy_config.hpp"
#include "hazard_config.hpp"
#include "power_up_config.hpp"
#include <variant>
#include <vector>

struct Wave {
    std::vector<ENEMY_TYPE> enemies;
};

struct Level {
    int last_wave;
    std::vector<HAZARD_TYPE> hazards;
    std::vector<POWER_UP_TYPE> powerups;
    std::vector<float> bells;
    std::vector<float> bubblers; // TODO: Change type once bubblers are further developed
    std::vector<int> swarms; // Secret for M3, leave it empty for now
};

const std::vector<Wave> WAVE_CONFIGS = {{{ENEMY_TYPE::GUARD}},
                                        {{ENEMY_TYPE::GUARD, ENEMY_TYPE::URCHIN}},
                                        {{ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING, ENEMY_TYPE::HOMING}}};

const std::vector<Level> LEVEL_CONFIGS = {
    {1, {}, {}, {}, {}},
    {3, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {500.f}, {0.f, 0.f}}
};

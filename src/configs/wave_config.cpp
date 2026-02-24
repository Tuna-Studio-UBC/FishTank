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
};

const std::vector<Wave> WAVE_CONFIGS = {{{ENEMY_TYPE::FOLLOWER}},
                                        {{ENEMY_TYPE::FOLLOWER, ENEMY_TYPE::URCHIN}},
                                        {{ENEMY_TYPE::SWARM, ENEMY_TYPE::SWARM, ENEMY_TYPE::SWARM, ENEMY_TYPE::SWARM}}};

const std::vector<Level> LEVEL_CONFIGS = {{{2, {HAZARD_TYPE::MINE}, {POWER_UP_TYPE::HEAL}, {500.f}}}};

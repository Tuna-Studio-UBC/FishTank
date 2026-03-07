#include "boss_config.hpp"
#include "attack_pattern.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

struct BossConfig {
    std::string name;
    Health health;
    float move_speed;
    float turn_speed;
    float detection_radius;
    BOSS_TYPE attack_patterns;
};

static const BossConfig BOSS_CONFIGS[boss_count] = {
    // Sharky
    {"Sharky", {2000, 2000}, 0.f, M_PI, 1000.f, BOSS_TYPE::SHARKY},

    // King Crabby
    {"King Crabby", {5000, 5000}, 0.f, M_PI, 1000.f, BOSS_TYPE::KING_CRABBY},

    // EEEEL
    {"EEEEL", {10000, 10000}, 0.f, M_PI, 1000.f, BOSS_TYPE::EEEEL},
};

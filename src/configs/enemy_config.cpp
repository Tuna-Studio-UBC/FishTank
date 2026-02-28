#include "enemy_config.hpp"
#include "../common.hpp"
#include "attack_pattern.hpp"

struct EnemyConfig {
    float move_speed;
    float turn_speed;
    float detection_radius;
    ATTACK_PATTERN attack_pattern;
};

static const EnemyConfig ENEMY_CONFIGS[enemy_count] = {
    // Turret
    {0.f, M_PI, 200.f, ATTACK_PATTERN::SINGLE},

    // Urchin
    {0.f, 0.f, 500.f, ATTACK_PATTERN::URCHIN},

    // Guard
    {50.f, M_PI / 2.f, 400.f, ATTACK_PATTERN::SINGLE},

    // Homing
    {50.f, M_PI / 2.f, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::SINGLE}
};

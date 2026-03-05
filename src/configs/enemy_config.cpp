#include "enemy_config.hpp"
#include "attack_pattern.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

struct EnemyConfig {
    Health health;
    float move_speed;
    float turn_speed;
    float detection_radius;
    ATTACK_PATTERN attack_pattern;
};

static const EnemyConfig ENEMY_CONFIGS[enemy_count] = {
    // Turret
    {{50, 50}, 0.f, M_PI, 500.f, ATTACK_PATTERN::SINGLE},

    // Urchin
    {{100, 100}, 0.f, 0.f, 500.f, ATTACK_PATTERN::URCHIN},

    // Guard
    {{50, 50}, 50.f, M_PI / 2.f, 600.f, ATTACK_PATTERN::SINGLE},

    // Homing
    {{30, 30}, 150.f, M_PI, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::RAPID}};

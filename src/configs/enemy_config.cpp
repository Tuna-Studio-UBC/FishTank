#include "enemy_config.hpp"
#include "attack_pattern.hpp"
#include "../common.hpp"

struct EnemyConfig
{
    float move_speed;
    float turn_speed;
    float detection_radius;
    ATTACK_PATTERN attack_pattern;
};

static const EnemyConfig ENEMY_CONFIGS[enemy_count] =
{
    // Turret
    { 0.f, 180.f, 200.f, ATTACK_PATTERN::SINGLE },

    // Urchin
    { 0.f, 0.f, 300.f ,ATTACK_PATTERN::URCHIN },

    // Follower
    { 50.f, 90.f, 200.f, ATTACK_PATTERN::SINGLE },

    // Swarm
    { 50.f, 90.f, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::SINGLE },

    // Patroller
    { 50.f, 90.f, 0.f, ATTACK_PATTERN::SINGLE }
}; 

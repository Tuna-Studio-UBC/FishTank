#include "attack_pattern.hpp"
#include <common.hpp>

struct AttackConfig
{
    float move_speed;
    float turn_speed;
    float damage;
};

static const AttackConfig ATTACK_CONFIGS[attack_count] =
{
    // Player
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 10.f },

    // Single
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 10.f },

    // Urchin
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 10.f }
};

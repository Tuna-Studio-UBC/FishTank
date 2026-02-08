#include "attack_pattern.hpp"
#include <common.hpp>

struct AttackConfig
{
    float move_speed;
    float turn_speed;
    float cooldown_ms;
    int damage;
    float lifespan_ms;
};

static const AttackConfig ATTACK_CONFIGS[attack_count] =
{
    // Player
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 2000.f, 10, 10000.f },

    // Single
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 2000.f, 10, 5000.f },

    // Urchin
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 5000.f, 10, 5000.f }
};

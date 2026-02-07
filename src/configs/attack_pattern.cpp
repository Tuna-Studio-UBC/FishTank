#include "attack_pattern.hpp"
#include <common.hpp>

struct AttackConfig
{
    float move_speed;
    float turn_speed;
    int damage;
    float lifespan_ms;
};

static const AttackConfig ATTACK_CONFIGS[attack_count] =
{
    // Player CHANGE IT BACK
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 100, 10000.f },

    // Single
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 100, 5000.f },

    // Urchin
    { PROJECTILE_SPEED, PROJECTILE_SPEED, 10, 5000.f }
};

#include "attack_pattern.hpp"
#include <common.hpp>

struct AttackConfig {
    int damage;
    float lifespan_ms;
    float projectile_speed;
    float cooldown_ms;
    vec3 projectile_tint = {1.f, .5f, .5f};
};

static const AttackConfig ATTACK_CONFIGS[attack_count] = {
    // Player
    {10, 10000.f, BASE_PROJECTILE_SPEED * 2.5, 2000.f, {1.f, 1.f, 1.f}},

    // Single
    {10, 5000.f, BASE_PROJECTILE_SPEED, 2000.f},

    // Urchin
    {10, 5000.f, BASE_PROJECTILE_SPEED, 5000.f}};

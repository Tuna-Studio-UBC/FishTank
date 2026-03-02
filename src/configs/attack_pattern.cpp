#include "attack_pattern.hpp"
#include "tinyECS/components.hpp"
#include <common.hpp>

struct AttackConfig {
    int damage;
    float lifespan_ms;
    float projectile_speed;
    float cooldown_ms;
    Tint projectile_tint = Tint({1.f, 1.f, 1.f});
};

static const AttackConfig ATTACK_CONFIGS[attack_count] = {
    // Player
    {10, 10000.f, BASE_PROJECTILE_SPEED * 2.5, 2000.f},

    // Single
    {10, 5000.f, BASE_PROJECTILE_SPEED, 2000.f, Tint({1.f, 0.5f, 0.25f})},

    // Rapid
    {10, 5000.f, BASE_PROJECTILE_SPEED * 1.5, 1000.f, Tint({1.f, 1.f, 0.f})},

    // Urchin
    {10, 5000.f, BASE_PROJECTILE_SPEED * 0.75, 4000.f, Tint({1.f, 0.f, 1.f})}};

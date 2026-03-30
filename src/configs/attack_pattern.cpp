#ifndef ATTACK_CONFIG
#define ATTACK_CONFIG

#include "attack_pattern.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

struct AttackConfig {
    ATTACK_EFFECT attack_effect;
    int damage;
    float lifespan_ms;
    float projectile_speed;
    float cooldown_ms;
    Tint projectile_tint = Tint({1.f, 1.f, 1.f});
    float projectile_scale = (float)PROJECTILE_SIZE_PX;
};

static const AttackConfig ATTACK_CONFIGS[attack_count] = {
    // Player
    {ATTACK_EFFECT::NORMAL, 10, 5000.f, BASE_PROJECTILE_SPEED * 2.5, 2000.f},

    /* --------------------------------- ENEMIES -------------------------------- */

    // Single
    {ATTACK_EFFECT::NORMAL, 10, 5000.f, BASE_PROJECTILE_SPEED, 2000.f, Tint({1.f, 0.5f, 0.25f})},

    // Rapid
    {ATTACK_EFFECT::NORMAL, 10, 3000.f, BASE_PROJECTILE_SPEED * 1.5, 1000.f, Tint({1.f, 1.f, 0.f})},

    // Toxic
    {ATTACK_EFFECT::TOXIC, 5, 3000.f, BASE_PROJECTILE_SPEED * 1.25, 2000.f, Tint({0.f, 0.f, 0.f})},

    // Machine gun
    {ATTACK_EFFECT::NORMAL, 1, 3000.f, BASE_PROJECTILE_SPEED * 3, 200.f, Tint({1.f, 1.f, 0.f}), 15.f},

    // Shotgun
    {ATTACK_EFFECT::NORMAL, 5, 2000.f, BASE_PROJECTILE_SPEED * 2, 3000.f, Tint({0.f, 1.f, 1.f})},

    // Urchin
    {ATTACK_EFFECT::NORMAL, 10, 10000.f, BASE_PROJECTILE_SPEED * 0.75, 3000.f, Tint({1.f, 0.f, 1.f})},

    // Havoc
    {ATTACK_EFFECT::NORMAL, 1, 3000.f, BASE_PROJECTILE_SPEED * 0.5f, 5000.f, Tint({1.f, 0.5f, 0.25f})},

    // Vengence
    {ATTACK_EFFECT::NORMAL, 1, 3000.f, BASE_PROJECTILE_SPEED * 1.5f, 2000.f, Tint({1.f, 0.5f, 0.25f})},

    /* --------------------------------- BOSSES --------------------------------- */

    // Sharky
    {ATTACK_EFFECT::NORMAL, 5, 3000.f, BASE_PROJECTILE_SPEED * 2, 100.f, Tint({0.2f, 0.9f, 1.f})},
    {ATTACK_EFFECT::NORMAL, 4, 5000.f, BASE_PROJECTILE_SPEED * 2.25, 100.f, Tint({0.2f, 0.9f, 1.f})},

    // King Crabby
    {ATTACK_EFFECT::NORMAL, 10, 5000.f, BASE_PROJECTILE_SPEED, 1000.f, Tint({1.f, 0.5f, 0.8f})},

    {ATTACK_EFFECT::TOXIC, 5, 3000.f, BASE_PROJECTILE_SPEED * 1.25, 2000.f, Tint({0.f, 0.f, 0.f}),
     PROJECTILE_SIZE_PX * 1.25f},

    {ATTACK_EFFECT::TOXIC, 8, 4000.f, BASE_PROJECTILE_SPEED, 250.f, Tint({0.f, 0.f, 0.f}), PROJECTILE_SIZE_PX * 1.25f},

    // EEEEL
    {ATTACK_EFFECT::NORMAL, 10, 10000.f, BASE_PROJECTILE_SPEED * 0.75, 4000.f, Tint({1.f, 0.f, 1.f})}};

#endif

#pragma once

#include "common.hpp"

enum class ATTACK_PATTERN {
    PLAYER = 0,
    SINGLE = PLAYER + 1,
    RAPID = SINGLE + 1,
    TOXIC = RAPID + 1,
    MACHINE_GUN = TOXIC + 1,
    SHOTGUN = MACHINE_GUN + 1,
    URCHIN = SHOTGUN + 1,
    ATTACK_COUNT = URCHIN + 1
};
const int attack_count = (int)ATTACK_PATTERN::ATTACK_COUNT;

enum class ATTACK_EFFECT { NORMAL = 0, TOXIC = NORMAL + 1, ATTACK_EFFECT_COUNT = TOXIC + 1 };
const int attack_effect_count = (int)ATTACK_EFFECT::ATTACK_EFFECT_COUNT;

struct AttackEffectConfig {
    float interval_ms;
    float duration_ms;
    vec3 damage_tint = {1.f, 0.25f, 0.25f};
};

static const AttackEffectConfig ATTACK_EFFECT_CONFIGS[attack_effect_count] = {
    // Normal
    {0.f, 0.f},

    // Toxic
    {400.f, 1600.f, {0.25f, 0.25f, 1.f}},
};

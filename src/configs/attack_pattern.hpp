#pragma once

#include "common.hpp"

#pragma region Patterns
// =================================================================================
// Patterns
// =================================================================================

enum class ATTACK_PATTERN {
    // Player
    PLAYER = 0,

    // Enemies
    SINGLE = PLAYER + 1,
    RAPID = SINGLE + 1,
    TOXIC = RAPID + 1,
    MACHINE_GUN = TOXIC + 1,
    SHOTGUN = MACHINE_GUN + 1,
    URCHIN = SHOTGUN + 1,
    HAVOC = URCHIN + 1,
    VENGENCE = HAVOC + 1,

    // Bosses
    SHARKY_NORMAL = VENGENCE + 1,
    SHARKY_WAVES = SHARKY_NORMAL + 1,

    KING_CRABBY_NORMAL = SHARKY_WAVES + 1,
    KING_CRABBY_TOXIC_A = KING_CRABBY_NORMAL + 1,
    KING_CRABBY_TOXIC_B = KING_CRABBY_TOXIC_A + 1,

    EEEEL_NORMAL = KING_CRABBY_TOXIC_B + 1,

    ATTACK_COUNT = EEEEL_NORMAL + 1
};
const int attack_count = (int)ATTACK_PATTERN::ATTACK_COUNT;

#pragma endregion Patterns

#pragma region Effects
// =================================================================================
// Effects
// =================================================================================

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

#pragma endregion Effects

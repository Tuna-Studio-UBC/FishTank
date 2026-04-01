#include "enemy_config.hpp"
#include "attack_pattern.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

#ifndef ENEMY_CONFIG
#define ENEMY_CONFIG

#pragma region Enemies
// =================================================================================
// Enemies
// =================================================================================

struct EnemyConfig {
    std::string name;
    Health health;
    float move_speed;
    float turn_speed;
    float detection_radius;
    ATTACK_PATTERN attack_pattern;
};

static const EnemyConfig ENEMY_CONFIGS[enemy_count] = {
    // Turret
    {"Turret", {50, 50}, 0.f, M_PI, 500.f, ATTACK_PATTERN::SINGLE},

    // Urchin
    {"Urchin", {100, 100}, 0.f, 0.f, 500.f, ATTACK_PATTERN::URCHIN},

    // Guard
    {"Crab", {50, 50}, 50.f, M_PI / 2.f, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::SINGLE},

    // Homing
    {"Isopod", {30, 30}, 150.f, M_PI, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::RAPID},

    // Swarm
    {"Mother", {50, 50}, 50.f, M_PI / 2.f, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::SINGLE},

    {"Child", {30, 30}, 150.f, M_PI, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::HAVOC}};

#pragma endregion Enemies

#pragma region Bosses
// =================================================================================
// Bosses
// =================================================================================

// TODO: Add rate of attack pattern switching
struct BossConfig {
    std::string name;
    Health health;
    float move_speed;
    float turn_speed;
    float detection_radius;
    std::vector<ATTACK_PATTERN> attack_patterns;
};

static const BossConfig BOSS_CONFIGS[boss_count] = {
    // Sharky
    {"Sharky",
     {400, 400},
     150.f,
     M_PI * 1.5f,
     (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX),
     {ATTACK_PATTERN::SHARKY_NORMAL, ATTACK_PATTERN::SHARKY_WAVES}},

    // King Crabby
    {"King Crabby",
     {1200, 1200},
     100.f,
     M_PI,
     (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX),
     {ATTACK_PATTERN::KING_CRABBY_NORMAL, ATTACK_PATTERN::KING_CRABBY_TOXIC_A, ATTACK_PATTERN::KING_CRABBY_TOXIC_B}},

    // EEEEL
    {"EEEEL",
     {500, 500},
     100.f,
     M_PI,
     (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX),
     {ATTACK_PATTERN::EEEEL_NORMAL}},
};

#pragma endregion Bosses

#endif

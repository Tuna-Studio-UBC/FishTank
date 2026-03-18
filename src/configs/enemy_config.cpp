#include "enemy_config.hpp"
#include "attack_pattern.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

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
    {"Crab", {50, 50}, 50.f, M_PI / 2.f, 600.f, ATTACK_PATTERN::SINGLE},

    // Homing
    {"Isopod", {30, 30}, 150.f, M_PI, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::RAPID},

    // Swarm
    {"Mother", {50, 50}, 50.f, M_PI / 2.f, 600.f, ATTACK_PATTERN::SINGLE},

    {"Child", {30, 30}, 150.f, M_PI, (float)std::max(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ATTACK_PATTERN::HAVOC}};

#pragma endregion Enemies

#pragma region Bosses
// =================================================================================
// Bosses
// =================================================================================

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
    {"Sharky", {500, 500}, 0.f, M_PI, 1000.f, {ATTACK_PATTERN::SHARKY1}},

    // King Crabby
    {"King Crabby", {2000, 2000}, 0.f, M_PI, 1000.f, {ATTACK_PATTERN::KING_CRABBY1}},

    // EEEEL
    {"EEEEL", {1000, 1000}, 0.f, M_PI, 1000.f, {ATTACK_PATTERN::EEEEL1}},
};

#pragma endregion Bosses

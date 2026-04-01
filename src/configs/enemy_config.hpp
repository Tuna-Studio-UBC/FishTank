#pragma once

#pragma region Enemies
// =================================================================================
// Enemies
// =================================================================================

enum class ENEMY_TYPE {
    TURRET = 0,          // stationary, detection, single shot
    URCHIN = TURRET + 1, // stationary, detection, multishot
    GUARD = URCHIN + 1,  // moving, single shot
    HOMING = GUARD + 1,  // moving, single shot
    MOTHER = HOMING + 1, // bezier, swarm leader
    CHILD = MOTHER + 1,  // swarm
    DASHER = CHILD + 1,  // moving, melee
    ENEMY_COUNT = DASHER + 1
};
const int enemy_count = (int)ENEMY_TYPE::ENEMY_COUNT;

#pragma endregion Enemies

#pragma region Bosses
// =================================================================================
// Bosses
// =================================================================================

enum class BOSS_TYPE { SHARKY = 0, KING_CRABBY = SHARKY + 1, EEEEL = KING_CRABBY + 1, BOSS_COUNT = EEEEL + 1 };
const int boss_count = (int)BOSS_TYPE::BOSS_COUNT;

#pragma endregion Bosses

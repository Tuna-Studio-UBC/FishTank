#pragma once

enum class ENEMY_TYPE {
    TURRET = 0,          // stationary, detection, single shot
    URCHIN = TURRET + 1, // stationary, detection, multishot
    GUARD = URCHIN + 1,  // moving, detection, single shot
    HOMING = GUARD + 1,  // moving, single shot
    MOTHER = HOMING + 1,
    CHILD = MOTHER + 1,
    ENEMY_COUNT = CHILD + 1
};
const int enemy_count = (int)ENEMY_TYPE::ENEMY_COUNT;

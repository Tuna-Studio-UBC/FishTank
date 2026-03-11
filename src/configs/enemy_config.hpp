#pragma once

enum class ENEMY_TYPE {
    TURRET = 0,          // stationary, detection, single shot
    URCHIN = TURRET + 1, // stationary, detection, multishot
    GUARD = URCHIN + 1,  // moving, detection, single shot
    HOMING = GUARD + 1,  // moving, single shot
    ENEMY_COUNT = HOMING + 1
};
const int enemy_count = (int)ENEMY_TYPE::ENEMY_COUNT;

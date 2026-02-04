#pragma once

enum class ENEMY_TYPE
{
    TURRET = 0,
    URCHIN = TURRET + 1,
    FOLLOWER = URCHIN + 1,
    SWARM = FOLLOWER + 1,
    PATROLLER = SWARM + 1,
    ENEMY_COUNT = PATROLLER + 1
};
const int enemy_count = (int)ENEMY_TYPE::ENEMY_COUNT;
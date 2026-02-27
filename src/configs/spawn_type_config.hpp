#pragma once

#define SPAWN_TYPE unsigned int

// Can store up to 32 different spawn types (unsigned int)
const enum SpawnType : SPAWN_TYPE {
    NONE = 0,
    ANY = 1,
    PLAYER = 2,
    ENEMY = 4,
    HAZARD = 8,
    AMMO = 16,
    POWERUP = 32,
    DISTRACTION = 64,
};

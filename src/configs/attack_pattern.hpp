#pragma once

enum class ATTACK_PATTERN
{
    PLAYER = 0,
    SINGLE = PLAYER + 1,
    URCHIN = SINGLE + 1,
    ATTACK_COUNT = URCHIN + 1
};
const int attack_count = (int)ATTACK_PATTERN::ATTACK_COUNT;
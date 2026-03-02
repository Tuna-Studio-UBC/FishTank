#pragma once

enum class ATTACK_PATTERN {
    PLAYER = 0,
    SINGLE = PLAYER + 1,
    RAPID = SINGLE + 1,
    URCHIN = RAPID + 1,
    ATTACK_COUNT = URCHIN + 1
};
const int attack_count = (int)ATTACK_PATTERN::ATTACK_COUNT;

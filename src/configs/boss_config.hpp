#pragma once

enum class BOSS_TYPE { SHARKY = 0, KING_CRABBY = SHARKY + 1, EEEEL = KING_CRABBY + 1, BOSS_COUNT = EEEEL + 1 };
const int boss_count = (int)BOSS_TYPE::BOSS_COUNT;

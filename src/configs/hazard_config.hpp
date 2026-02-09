#pragma once

enum class HAZARD_TYPE { MINE = 0, ANEMONE = MINE + 1, HAZARD_TYPE_COUNT = ANEMONE + 1 };
const int hazard_count = (int)HAZARD_TYPE::HAZARD_TYPE_COUNT;

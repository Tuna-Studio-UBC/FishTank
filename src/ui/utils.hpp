#pragma once
#include <algorithm>

#include "imgui.h"

float lerp(float a, float b, float t);

ImU32 colour_lerp(ImU32 col_a, ImU32 col_b, float t);

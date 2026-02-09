#include "utils.hpp"

float lerp(float a, float b, float t) { return a + t * (b - a); }

ImU32 colour_lerp(ImU32 col_a, ImU32 col_b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    ImVec4 vec_a = ImGui::ColorConvertU32ToFloat4(col_a);
    ImVec4 vec_b = ImGui::ColorConvertU32ToFloat4(col_b);
    ImVec4 new_colour = ImVec4(lerp(vec_a.x, vec_b.x, t), lerp(vec_a.y, vec_b.y, t), lerp(vec_a.z, vec_b.z, t),
                               lerp(vec_a.w, vec_b.w, t));

    return ImGui::ColorConvertFloat4ToU32(new_colour);
}

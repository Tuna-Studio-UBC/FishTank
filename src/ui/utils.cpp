#include "utils.hpp"

float lerp(float a, float b, float t) { return a + t * (b - a); }

ImU32 colour_lerp(ImU32 col_a, ImU32 col_b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    ImVec4 vec_a = ImGui::ColorConvertU32ToFloat4(col_a);
    ImVec4 vec_b = ImGui::ColorConvertU32ToFloat4(col_b);

    // RGB to HSV
    ImGui::ColorConvertRGBtoHSV(vec_a.x, vec_a.y, vec_a.z, vec_a.x, vec_a.y, vec_a.z);
    ImGui::ColorConvertRGBtoHSV(vec_b.x, vec_b.y, vec_b.z, vec_b.x, vec_b.y, vec_b.z);

    ImVec4 new_colour = ImVec4(lerp(vec_a.x, vec_b.x, t), lerp(vec_a.y, vec_b.y, t), lerp(vec_a.z, vec_b.z, t),
                               lerp(vec_a.w, vec_b.w, t));

    // HSV to RGB
    ImGui::ColorConvertHSVtoRGB(new_colour.x, new_colour.y, new_colour.z, new_colour.x, new_colour.y, new_colour.z);

    return ImGui::ColorConvertFloat4ToU32(new_colour);
}

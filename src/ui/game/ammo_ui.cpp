#include "game_ui.hpp"

namespace {
ImU32 get_ammo_bar_colour(float ammo_ratio) {
    if (ammo_ratio <= 0.3f) {
        return IM_COL32(220, 68, 68, 255);
    }

    if (ammo_ratio <= 0.6f) {
        return IM_COL32(240, 200, 70, 255);
    }

    return IM_COL32(0, 107, 189, 255);
}
} // namespace

void GameUI::draw_ammo() {
    float bar_width = 300;
    float bar_height = 10;
    float padding = 2;
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * .66f, WINDOW_HEIGHT_PX * .9f), ImGuiCond_Once, ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Ammo Bar", nullptr, window_flags);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    Entity e = registry.players.entities[0];

    if (registry.fireControls.has(e)) {
        FireControl &fc = registry.fireControls.get(e);
        float ratio = 0.f;
        if (fc.max_ammo > 0) {
            ratio = std::clamp((float)fc.ammo / (float)fc.max_ammo, 0.f, 1.f);
        }
        ImVec2 rect_min_filled = ImVec2(pos.x + padding, pos.y + padding);
        ImVec2 rect_max_filled = ImVec2(pos.x + (size.x - padding) * ratio, pos.y + (size.y - padding));
        ImU32 colour = get_ammo_bar_colour(ratio);
        draw_list->AddRectFilled(rect_min_filled, rect_max_filled, colour);
        const std::string ammo_text = "AMMO: " + std::to_string(fc.ammo) + "/" + std::to_string(fc.max_ammo);
        ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), "%s", ammo_text.c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

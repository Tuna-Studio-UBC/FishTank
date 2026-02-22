#include "game_ui.hpp"

const ImVec4 UI_COLOUR = ImVec4(0.f, 0.418, 0.741f, 1.f); // Should this be here?

void GameUI::draw_ammo() {
    float bar_width = 300;
    float bar_height = 10;
    float padding = 2;
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * .66f, WINDOW_HEIGHT_PX * .9f), ImGuiCond_Once, ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Ammo Bar", nullptr, window_flags);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    Entity e = registry.players.entities[0];

    if (registry.fireControl.has(e)) {
        FireControl &fc = registry.fireControl.get(e);
        float ratio = (fc.ammo > 0) ? (float)fc.ammo / (float)fc.max_ammo : 0.f;
        ImVec2 rect_min_filled = ImVec2(pos.x + padding, pos.y + padding);
        ImVec2 rect_max_filled = ImVec2(pos.x + (size.x - padding) * ratio, pos.y + (size.y - padding));
        ImU32 colour = ImGui::ColorConvertFloat4ToU32(UI_COLOUR);
        draw_list->AddRectFilled(rect_min_filled, rect_max_filled, colour);
        ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), std::string("AMMO: " + std::to_string(fc.ammo) + "/" + std::to_string(fc.max_ammo)).c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

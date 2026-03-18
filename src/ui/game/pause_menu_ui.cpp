#include "game_ui.hpp"

#include <iomanip>
#include <sstream>

void GameUI::draw_pause_menu() {
    Progress &progress = registry.progresses.components[0];
    if (!progress.is_paused || progress.is_tutorial_step_called || progress.is_upgrade_menu_called)
        return;

    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Pause Menu", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 150), 0.f, 0.f);

    ImGui::PushFont(io.Fonts->Fonts[1]);
    const char *text = "GAME PAUSED";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 150.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[2]);
    text = "Press 'P' to resume.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
}

#include "game_ui.hpp"

void GameUI::draw_score() {
    float bar_width = 240;
    float bar_height = 30;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
    ImGui::SetNextWindowPos(ImVec2(bar_width / 2, bar_height / 2), ImGuiCond_Once, ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBackground;

    ImGui::Begin("Score UI", nullptr, window_flags);

    Scoring &score = registry.scoring.components[0];

    ImGui::PushFont(io.Fonts->Fonts[3]);
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), std::string("Score: " + std::to_string(score.crackers_earned * 1000)).c_str());
    ImGui::PopFont();

    ImGui::End();
}

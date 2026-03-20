#include "game_ui.hpp"

void GameUI::draw_wallet() {
    float bar_width = 240;
    float bar_height = 30;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 120 - 12, WINDOW_HEIGHT_PX * .37f), ImGuiCond_Once,
                            ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 0, .3f));
    ImGui::Begin("Wallet UI", nullptr, window_flags);

    Entity e = registry.players.entities[0];
    Wallet &wc = registry.wallets.get(e);

    ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), std::string("Crackers: " + std::to_string(wc.crackers)).c_str());

    ImGui::End();
    ImGui::PopStyleColor();
}

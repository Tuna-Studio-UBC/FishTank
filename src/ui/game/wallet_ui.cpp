#include "game_ui.hpp"

#include "systems/level/level_system.hpp"

void GameUI::draw_wallet() {
    float bar_width = 240;
    float bar_height = 52;
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
    const std::string crackers_text = "Crackers: " + std::to_string(wc.crackers);
    ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), "%s", crackers_text.c_str());

    if (level_system != nullptr && !level_system->is_in_tutorial()) {
        const int current_wave = level_system->get_current_wave();
        const int total_waves = level_system->get_total_waves_for_current_level();
        if (current_wave >= 0 && total_waves > 0) {
            const std::string wave_text =
                "Wave " + std::to_string(current_wave + 1) + " / " + std::to_string(total_waves);
            ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), "%s", wave_text.c_str());
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

#include "game_ui.hpp"

#include <iomanip>
#include <sstream>

void GameUI::draw_fps_counter(WorldSystem *world_system) {
    if (!world_system->get_show_fps())
        return;

    ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("FPS Counter", nullptr, window_flags);

    std::stringstream fps;
    fps << std::fixed << std::setprecision(2) << world_system->get_fps();
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), std::string("FPS: " + fps.str()).c_str());

    ImGui::End();
    ImGui::PopStyleColor();
}

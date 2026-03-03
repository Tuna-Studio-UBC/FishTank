#include "debug_ui.hpp"

void DebugUI::draw_respawn(WorldSystem *world_system) {
    ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Respawn", nullptr, window_flags);

    if (ImGui::Button(("Respawn " + registry.players.entities[0].name_id()).c_str())) {
        world_system->respawn_player();
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

#include "game_ui.hpp"

#include <iomanip>
#include <sstream>
// prevent linker error
GameUI game_ui;

void GameUI::draw_cursor() {
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    ImGui::Begin("Cursor", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 mouse_pos = io.MousePos;
    ImVec2 cursor_size = ImVec2(48.f, 48.f);

    int offset_id = last_tileset_id + 1;
    int id = (int)TEXTURE_ASSET_ID::TEXTURE_CURSOR + offset_id;
    float is_click = ImGui::IsMouseDown(0) ? 0.5f : 0.f;
    ImGui::GetForegroundDrawList()->AddImage((ImTextureID)id,
                                             ImVec2(mouse_pos.x - cursor_size.x / 2, mouse_pos.y - cursor_size.y / 2),
                                             ImVec2(mouse_pos.x + cursor_size.x / 2, mouse_pos.y + cursor_size.y / 2),
                                             ImVec2(is_click + 0.f, 0.f), ImVec2(is_click + 0.5f, 1.f));

    ImGui::End();
}

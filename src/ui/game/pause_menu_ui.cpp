#include "game_ui.hpp"

#include <iomanip>
#include <sstream>

namespace {
std::string format_upgrade_line(const char *label, unsigned int level) {
    std::ostringstream ss;
    ss << label << ": " << level;
    return ss.str();
}

void draw_separator(ImDrawList *draw_list, float x, float y, float width) {
    draw_list->AddLine(ImVec2(x, y), ImVec2(x + width, y), IM_COL32(180, 180, 180, 70), 1.5f);
}
} // namespace

void GameUI::draw_pause_menu() {
    Progress &progress = registry.progresses.components[0];
    if (!progress.is_paused || progress.is_tutorial_step_called || progress.is_upgrade_menu_called)
        return;

    if (registry.players.entities.empty())
        return;

    Entity player = registry.players.entities[0];
    if (!registry.wallets.has(player) || !registry.upgrades.has(player))
        return;

    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Pause Menu", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 150), 0.f, 0.f);

    const ImVec2 card_size(920.f, 520.f);
    const ImVec2 card_min((WINDOW_WIDTH_PX - card_size.x) * 0.5f, (WINDOW_HEIGHT_PX - card_size.y) * 0.5f);
    const ImVec2 card_max(card_min.x + card_size.x, card_min.y + card_size.y);
    draw_list->AddRectFilled(card_min, card_max, IM_COL32(18, 18, 18, 220), 16.f, 0);
    draw_list->AddRect(card_min, card_max, IM_COL32(215, 215, 215, 70), 16.f, 0, 2.0f);

    ImGui::PushFont(io.Fonts->Fonts[1]);
    const char *text = "GAME PAUSED";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, card_min.y + 34.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[2]);
    text = "Press 'Esc' to resume.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, card_min.y + 100.f));
    ImGui::TextColored(ImVec4(.85f, .85f, .85f, 1.f), "%s", text);

    Wallet &wallet = registry.wallets.get(player);
    Upgrades &upgrades = registry.upgrades.get(player);

    const float left_x = card_min.x + 56.f;
    const float left_width = 350.f;
    const float right_x = card_min.x + 470.f;
    const float right_width = card_max.x - right_x - 56.f;
    const float section_y = card_min.y + 170.f;
    const float row_gap = 42.f;

    std::string state_line = format_upgrade_line("Attack Upgrade", upgrades.upgrade_instances[UPGRADE_TYPE::ATTACK].level);
    ImGui::SetCursorPos(ImVec2(left_x, section_y));
    ImGui::TextColored(ImVec4(.92f, .92f, .92f, 1.f), "%s", state_line.c_str());

    state_line = format_upgrade_line("Ammo Upgrade", upgrades.upgrade_instances[UPGRADE_TYPE::MAX_AMMO].level);
    ImGui::SetCursorPos(ImVec2(left_x, section_y + row_gap));
    ImGui::TextColored(ImVec4(.92f, .92f, .92f, 1.f), "%s", state_line.c_str());

    state_line = format_upgrade_line("HP Upgrade", upgrades.upgrade_instances[UPGRADE_TYPE::MAX_HP].level);
    ImGui::SetCursorPos(ImVec2(left_x, section_y + row_gap * 2.f));
    ImGui::TextColored(ImVec4(.92f, .92f, .92f, 1.f), "%s", state_line.c_str());

    draw_separator(draw_list, left_x, section_y + 146.f, left_width);

    state_line = "Wallet: " + std::to_string(wallet.crackers) + " crackers";
    ImGui::SetCursorPos(ImVec2(left_x, section_y + 188.f));
    ImGui::TextColored(ImVec4(.92f, .92f, .92f, 1.f), "%s", state_line.c_str());

    ImGui::SetCursorPos(ImVec2(left_x, section_y + 188.f + row_gap));
    ImGui::Checkbox("Use Alt Controls (WASD)", input_system->get_input_mode());

    text = "MASTER VOLUME";
    ImGui::SetCursorPos(ImVec2(right_x, section_y));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);

    int master_volume = audio_system != nullptr ? audio_system->get_master_volume() : MIX_MAX_VOLUME;
    int volume_percent = (master_volume * 100) / MIX_MAX_VOLUME;
    std::string volume_label = std::to_string(volume_percent) + "%";
    ImGui::SetCursorPos(ImVec2(right_x, section_y + 52.f));
    ImGui::TextColored(ImVec4(.92f, .92f, .92f, 1.f), "%s", volume_label.c_str());

    ImGui::SetCursorPos(ImVec2(right_x, section_y + 95.f));
    ImGui::SetNextItemWidth(right_width);
    if (ImGui::SliderInt("##pause_master_volume", &master_volume, 0, MIX_MAX_VOLUME, "")) {
        if (audio_system != nullptr)
            audio_system->set_master_volume(master_volume);
    }

    ImGui::SetCursorPos(ImVec2(right_x, card_max.y - 110.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.18f, 0.18f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.62f, 0.22f, 0.22f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.38f, 0.12f, 0.12f, 1.f));
    if (ImGui::Button("Quit Game", ImVec2(right_width, 48.f))) {
        GLFWwindow *window = glfwGetCurrentContext();
        if (window != nullptr)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
}

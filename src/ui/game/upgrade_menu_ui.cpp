#include "game_ui.hpp"

#include <iomanip>
#include <sstream>

namespace {
const char *UPGRADE_NAMES[UPGRADE_TYPE::UPGRADE_COUNT] = {"Attack", "Max Ammo", "Max HP"};
const char *UPGRADE_DESCRIPTIONS[UPGRADE_TYPE::UPGRADE_COUNT] = {"Increase damage", "Carry more ammo",
                                                                 "Raise max health"};
const ImVec4 UPGRADE_COLOURS[UPGRADE_TYPE::UPGRADE_COUNT] = {
    ImVec4(0.84f, 0.35f, 0.24f, 1.f), ImVec4(0.20f, 0.53f, 0.88f, 1.f), ImVec4(0.22f, 0.70f, 0.38f, 1.f)};

std::string get_upgrade_bonus_label(UPGRADE_TYPE type) {
    switch (type) {
    case UPGRADE_TYPE::ATTACK:
        return "damage";
    case UPGRADE_TYPE::MAX_AMMO:
        return "ammo";
    case UPGRADE_TYPE::MAX_HP:
        return "hp";
    default:
        return "bonus";
    }
}

int get_total_bonus(const UpgradeInstance &upgrade) { 
    return upgrade.config.bonus[upgrade.level]; 
}

void draw_upgrade_card(UpgradeSystem *upgrade_system, Entity player, UPGRADE_TYPE type, const ImVec2 &card_size,
                       const ImVec2 &title_size) {
    UpgradeInstance &upgrade = registry.upgrades.get(player).upgrade_instances[type];
    Wallet &wallet = registry.wallets.get(player);
    const bool is_maxed = upgrade.level >= upgrade.config.max_levels - 1;
    const bool can_afford = !is_maxed && upgrade_system != nullptr && upgrade_system->can_afford_upgrade(type);
    const unsigned int next_level = std::min(upgrade.level + 1, upgrade.config.max_levels - 1);
    const unsigned int max_upgrade_level = upgrade.config.max_levels - 1;
    const int current_bonus = get_total_bonus(upgrade);
    const bool show_needed_text = !is_maxed && !can_afford && upgrade.config.cost[next_level] > (int)wallet.crackers;

    ImGui::PushID((int)type);
    ImGui::BeginChild("upgrade_card", card_size, true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    const ImVec2 min = ImGui::GetWindowPos();
    const ImVec2 max = ImVec2(min.x + card_size.x, min.y + card_size.y);
    ImU32 accent = ImGui::ColorConvertFloat4ToU32(UPGRADE_COLOURS[type]);
    draw_list->AddRect(min, max, accent, 10.f, 0, 3.f);

    ImGui::PushStyleColor(ImGuiCol_Text, UPGRADE_COLOURS[type]);
    ImGui::SetCursorPosX((card_size.x - title_size.x) * 0.5f);
    ImGui::Text("%s", UPGRADE_NAMES[type]);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::TextWrapped("%s", UPGRADE_DESCRIPTIONS[type]);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Level %u / %u", upgrade.level, max_upgrade_level);
    ImGui::Spacing();

    for (unsigned int level = 0; level < max_upgrade_level; ++level) {
        if (level > 0)
            ImGui::SameLine();

        const bool filled = level < upgrade.level;
        ImGui::TextColored(filled ? UPGRADE_COLOURS[type] : ImVec4(0.45f, 0.45f, 0.45f, 1.f), filled ? "[#]" : "[ ]");
    }

    ImGui::Spacing();
    ImGui::Text("Current: +%d %s", current_bonus, get_upgrade_bonus_label(type).c_str());

    if (is_maxed) {
        ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.70f, 1.f), "Next: MAXED");
        ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.70f, 1.f), "Cost: ---");
    } else {
        ImGui::Text("Next: +%d %s", upgrade.config.bonus[next_level], get_upgrade_bonus_label(type).c_str());
        ImGui::Text("Cost: %d crackers", upgrade.config.cost[next_level]);
    }

    ImGui::Spacing();
    ImGui::SetCursorPosY(card_size.y - (show_needed_text ? 80.f : 56.f));

    if (is_maxed) {
        ImGui::BeginDisabled();
        ImGui::Button("Maxed Out", ImVec2(-1.f, 38.f));
        ImGui::EndDisabled();
    } else if (!can_afford) {
        ImGui::BeginDisabled();
        ImGui::Button("Not Enough Crackers", ImVec2(-1.f, 38.f));
        ImGui::EndDisabled();
        const int needed = upgrade.config.cost[next_level] - (int)wallet.crackers;
        if (show_needed_text && needed > 0) {
            ImGui::TextColored(ImVec4(0.85f, 0.50f, 0.50f, 1.f), "Need %d more crackers", needed);
        }
    } else if (ImGui::Button("Upgrade", ImVec2(-1.f, 38.f))) {
        upgrade_system->try_upgrade_player(type);
    }

    ImGui::EndChild();
    ImGui::PopID();
}
} // namespace

void GameUI::set_upgrade_system(UpgradeSystem *upgrade_system) { this->upgrade_system = upgrade_system; }

void GameUI::set_level_system(LevelSystem *level_system) { this->level_system = level_system; }

void GameUI::set_audio_system(AudioSystem *audio_system) {
    this->audio_system = audio_system;
}

void GameUI::set_input_system(PlayerInputSystem *input_system) { this->input_system = input_system; }

void GameUI::draw_upgrade_menu() {
    Progress &progress = registry.progresses.components[0];
    if (!(progress.is_paused && progress.is_upgrade_menu_called))
        return;

    if (registry.players.entities.empty())
        return;

    Entity player = registry.players.entities[0];
    if (!registry.upgrades.has(player) || !registry.wallets.has(player))
        return;

    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Upgrade Menu", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 240), 0.f, 0.f);

    ImGui::PushFont(io.Fonts->Fonts[1]);
    const char *text = "UPGRADE STATION";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, 100.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[2]);
    Wallet &wallet = registry.wallets.get(player);
    std::string crackers_text = "Crackers: " + std::to_string(wallet.crackers);
    text_size = ImGui::CalcTextSize(crackers_text.c_str());
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, 170.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", crackers_text.c_str());

    const char *subtitle = "Choose your next upgrade before diving into the next level";
    text_size = ImGui::CalcTextSize(subtitle);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, 205.f));
    ImGui::TextColored(ImVec4(.85f, .85f, .85f, 1.f), "%s", subtitle);

    const float card_width = 300.f;
    const float card_height = 340.f;
    const float spacing = 28.f;
    const float total_width = card_width * 3.f + spacing * 2.f;
    float start_x = (WINDOW_WIDTH_PX - total_width) * 0.5f;
    float y = 270.f;

    for (int i = 0; i < UPGRADE_TYPE::UPGRADE_COUNT; ++i) {
        UPGRADE_TYPE type = static_cast<UPGRADE_TYPE>(i);
        ImGui::SetCursorPos(ImVec2(start_x + i * (card_width + spacing), y));
        const ImVec2 title_size = ImGui::CalcTextSize(UPGRADE_NAMES[i]);
        draw_upgrade_card(upgrade_system, player, type, ImVec2(card_width, card_height), title_size);
    }

    text = "Press 1 / 2 / 3 to upgrade, or click a card button. Press Esc to continue.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX - 90.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
}

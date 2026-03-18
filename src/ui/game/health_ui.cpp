#include "game_ui.hpp"

#include "systems/player/camera_system.hpp"

void GameUI::draw_healthbar() {
    float bar_width = 300;
    float bar_height = 10;
    float padding = 2;
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * .33f, WINDOW_HEIGHT_PX * .9f), ImGuiCond_Once, ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
    ImGui::Begin("Health Bar", nullptr, window_flags);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImU32 colour_start = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
    ImU32 colour_end = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 1.f, 0.f, 1.f));

    Entity e = registry.players.entities[0];

    if (registry.healths.has(e)) {
        Health &h = registry.healths.get(e);
        float ratio = (h.max_hp > 0) ? (float)h.hp / (float)h.max_hp : 0.f;
        ImVec2 rect_min_filled = ImVec2(pos.x + padding, pos.y + padding);
        ImVec2 rect_max_filled = ImVec2(pos.x + (size.x - padding) * ratio, pos.y + (size.y - padding));
        ImU32 colour = colour_lerp(colour_start, colour_end, ratio);
        draw_list->AddRectFilled(rect_min_filled, rect_max_filled, colour);

        ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f),
                           std::string("HP: " + std::to_string(h.hp) + "/" + std::to_string(h.max_hp)).c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

void GameUI::draw_enemy_healthbars() {
    float bar_width = 80;
    float bar_height = 8;
    float bar_offset_y = 20;
    float padding = 2;
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

    ImGui::Begin("Enemy Healthbars", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImU32 colour_start = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
    ImU32 colour_end = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 1.f, 0.f, 1.f));

    ImGui::PushFont(io.Fonts->Fonts[3]);

    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "\nENEMIES:");
    for (Entity e : registry.enemies.entities) {
        Motion &m = registry.motions.get(e);
        Health &h = registry.healths.get(e);
        float ratio = (h.max_hp > 0) ? (float)h.hp / (float)h.max_hp : 0.f;

        ImGui::TextColored(
            ImVec4(1.f, 1.f, 1.f, 1.f),
            std::string(e.name() + ": " + std::to_string(h.hp) + "/" + std::to_string(h.max_hp)).c_str());

        // Skip bosses, they have their own health bar
        if (registry.bosses.has(e))
            continue;

        vec2 screen_pos = CameraSystem::get_screen_pos(m.position);
        float margin = TILE_SIZE_PX / 2;
        if (screen_pos.x < 0 - margin || screen_pos.x > WINDOW_WIDTH_PX + margin || screen_pos.y < 0 - margin ||
            screen_pos.y > WINDOW_HEIGHT_PX + margin)
            continue;

        ImVec2 rect_min =
            ImVec2(screen_pos.x - bar_width / 2 + padding, screen_pos.y - bar_height + padding - bar_offset_y);
        ImVec2 rect_max = ImVec2(rect_min.x + bar_width - padding, rect_min.y + bar_height - padding);
        draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(50, 50, 50, 150));

        ImVec2 health_rect_min =
            ImVec2(screen_pos.x - bar_width / 2 + padding, screen_pos.y - bar_height + padding - bar_offset_y);
        ImVec2 health_rect_max =
            ImVec2(health_rect_min.x + bar_width * ratio - padding, health_rect_min.y + bar_height - padding);
        ImU32 colour = colour_lerp(colour_start, colour_end, ratio);
        draw_list->AddRectFilled(health_rect_min, health_rect_max, colour);
    }

    ImGui::PopFont();
    ImGui::End();
}

void GameUI::draw_boss_healthbars() {
    float bar_width = WINDOW_WIDTH_PX / 3.f;
    float bar_height = 40.f;
    float padding = 2;
    ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * .5f, WINDOW_HEIGHT_PX * .15f), ImGuiCond_Once, ImVec2(.5f, .5f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

    ImGui::Begin("Boss Health Bar", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImGui::PushFont(io.Fonts->Fonts[3]);

    ImU32 colour_start = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
    ImU32 colour_end = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 1.f, 0.f, 1.f));

    for (Entity e : registry.bosses.entities) {
        if (registry.healths.has(e)) {
            Health &h = registry.healths.get(e);

            ImVec2 rect_min = ImVec2(pos.x, pos.y);
            ImVec2 rect_max = ImVec2(pos.x + size.x, pos.y + size.y);
            draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(51, 51, 51, 230));

            float ratio = (h.max_hp > 0) ? (float)h.hp / (float)h.max_hp : 0.f;
            ImVec2 health_rect_min = ImVec2(pos.x + padding, pos.y + padding);
            ImVec2 health_rect_max = ImVec2(pos.x + (size.x - padding) * ratio, pos.y + (size.y - padding));
            ImU32 colour = colour_lerp(colour_start, colour_end, ratio);
            draw_list->AddRectFilled(health_rect_min, health_rect_max, colour);

            ImGui::TextColored(
                ImVec4(0.f, 0.f, 0.f, 1.f),
                std::string(e.name() + " - " + std::to_string(h.hp) + "/" + std::to_string(h.max_hp)).c_str());
        }
    }

    ImGui::PopFont();
    ImGui::End();
}

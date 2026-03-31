#include "tutorial_ui.hpp"

#include <spdlog/spdlog.h>

void TutorialUI::draw_tutorial_step() {
    Progress &progress = registry.progresses.components[0];

    // Skip if not in tutorial
    if (!progress.is_tutorial)
        return;

    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("Tutorial", nullptr, window_flags);

    if (progress.is_tutorial_step_called) {
        Tutorial part = progress.tutorial_step_id;
        assert((part > 0 && part <= progress.tutorial_steps.size()) && "Invalid tutorial step!");
        TutorialStep &tutorial = progress.tutorial_steps[part - 1];

        switch (part) {
        case Tutorial::MOVEMENT:
            draw_tutorial_movement(tutorial);
            break;
        case Tutorial::INTERACT:
            draw_tutorial_interactables(tutorial);
            break;
        case Tutorial::COMBAT:
            draw_tutorial_combat(tutorial);
            break;
        }

        ImGuiIO &io = ImGui::GetIO();
        ImGui::PushFont(io.Fonts->Fonts[2]);
        const char *text = "Press ENTER or CLICK to continue...";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImVec2 padding = ImVec2(20.f, 20.f);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX - text_size.x - padding.x, WINDOW_HEIGHT_PX - text_size.y - padding.y));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    }

    if (!progress.is_transitioning_level)
        draw_tutorial_skip_button();

    ImGui::End();
    ImGui::PopStyleColor();
}

void TutorialUI::draw_tutorial_skip_button() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[3]);
    const char *text = "Skip Tutorial >>";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImVec2 padding = ImVec2(20.f, 60.f);
    ImVec2 button_pos = ImVec2((WINDOW_WIDTH_PX - text_size.x) / 2 + padding.x, padding.y);
    ImGui::SetCursorPos(button_pos);
    ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), text);
    ImGui::PopFont();

    if (ImGui::IsMouseHoveringRect(button_pos, ImVec2(button_pos.x + text_size.x, button_pos.y + text_size.y), false)) {
        auto &progress = registry.progresses.components[0];

        if (ImGui::IsMouseClicked(0) &&
            (progress.is_tutorial && (!progress.is_paused || progress.is_tutorial_step_called))) {
            spdlog::info("Skipping tutorial...");
            progress.is_tutorial_step_called = false;
            progress.is_paused = false;
            progress.trigger_skip_wave = true;
            progress.force_skip_tutorial = true;
        }
    }
}

void TutorialUI::draw_tutorial_movement(TutorialStep tutorial) {
    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 180), 0.f, 0.f);

    ImGui::PushFont(io.Fonts->Fonts[1]);

    const char *text = "Welcome to FishTank!";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

    ImGui::PopFont();
}

void TutorialUI::draw_tutorial_interactables(TutorialStep tutorial) {
    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 180), 0.f, 0.f);

    int current_step = tutorial.current_step;
    if (current_step == 1) {
        ImGui::PushFont(io.Fonts->Fonts[1]);
        const char *text = "Interactables";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 200.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();

        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "In the game you'll encounter:";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 100.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

        text = "Hazards";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 50.f));
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), text);

        text = "Powerups/Health packs";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), text);
 
        text = "Bubblers (to refill your ammo)";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(0.95f, 0.65f, 0.38f, 1.f), text);

        text = "Move around to test some (or all) in this area!";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 100.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    }
}


void TutorialUI::draw_tutorial_combat(TutorialStep tutorial) {
    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 180), 0.f, 0.f);

    ImGui::PushFont(io.Fonts->Fonts[1]);
    const char *text = "COMBAT";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 200.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[2]);
    text = "Move the mouse to aim";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 100.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

    text = "Press 1 / 2\nto switch weapons";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(
        ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2 - 200.f, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

    text = "Left / Right (mouse)\nto switch attacks";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(
        ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2 + 200.f, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

    text = "Clear this area to start playing for real!";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 100.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();
}

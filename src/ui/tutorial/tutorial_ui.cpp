#include "tutorial_ui.hpp"

#include <spdlog/spdlog.h>

void TutorialUI::draw_tutorial_step() {
    Progress &progress = registry.progresses.components[0];
    if (!progress.is_tutorial_step_called)
        return;

    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("Tutorial", nullptr, window_flags);

    Tutorial part = progress.tutorial_step_id;
    assert((part > 0 && part <= progress.tutorial_steps.size()) && "Invalid tutorial step!");
    TutorialStep &tutorial = progress.tutorial_steps[part - 1];

    switch (part) {
    case Tutorial::MOVEMENT:
        draw_tutorial_movement(tutorial);
        break;
    case Tutorial::HAZARDS:
        draw_tutorial_hazards(tutorial);
        break;
    case Tutorial::HEALING:
        draw_tutorial_healing(tutorial);
        break;
    case Tutorial::AMMUNITION:
        draw_tutorial_ammo(tutorial);
        break;
    case Tutorial::COMBAT:
        draw_tutorial_combat(tutorial);
        break;
    case Tutorial::UPGRADE_SYSTEM:
        draw_tutorial_upgrade_system(tutorial);
        break;
    }

    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[2]);
    const char *text = "Press ENTER or CLICK to continue...";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImVec2 padding = ImVec2(20.f, 20.f);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX - text_size.x - padding.x, WINDOW_HEIGHT_PX - text_size.y - padding.y));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
}

void TutorialUI::draw_tutorial_movement(TutorialStep tutorial) {
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

        const char *text = "Welcome to FishTank!";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

        ImGui::PopFont();
    } else if (current_step == 2) {
        ImGui::PushFont(io.Fonts->Fonts[1]);
        const char *text = "Here's how to move around";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 200.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();

        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "W = swim FORWARD";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

        text = "A = turn LEFT";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2 - 200.f, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

        text = "D = turn RIGHT";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2 + 200.f, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);

        text = "S = turn 180 DEGREES";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 150.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    }
}

void TutorialUI::draw_tutorial_hazards(TutorialStep tutorial) {
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
        const char *text = "WATCH OUT!";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 200.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();

        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "In the game you'll encounter some hazards:\n\nMines (Blinking red obstacles).\nAnemones (Purple stinging tentacles)";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    }
}

void TutorialUI::draw_tutorial_healing(TutorialStep tutorial) {
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
        const char *text = "MECHANIC: POWER UPS";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 150.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();

        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "You'll also encounter some health packs and other powerups.\nJust swim over them to get their effects!\n\nFeathers give a temporary speed boost\nThe yellow container gives a temporary damage boost\nHealth packs replenish health (Not consumed at full health)";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    }
}

void TutorialUI::draw_tutorial_ammo(TutorialStep tutorial) {
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
        const char *text = "RECOVERY: AMMUNITION";
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 150.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();

        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "Bubblers will replenish your ammo.\n\nSince you only have a limited amount of ammo, be "
               "sure to recharge whenever possible!\n\nJust swim over them to recover some ammo!";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
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
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 - 150.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
    ImGui::PopFont();

    int current_step = tutorial.current_step;
    if (current_step == 1) {
        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "Uh oh! We've got some enemies ahead of us!\n\nSee if you can take 'em out!\n\nPoint and click with "
               "your mouse to fire some bubbles!";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    } else if (current_step == 2) {
        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "But be careful, they can fire back!";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), text);
        ImGui::PopFont();
    } else if (current_step == 3) {
        ImGui::PushFont(io.Fonts->Fonts[2]);
        text = "You have two weapon modes:\n\n"
               "GUN 1 (Press 1): Regular bouncy bubbles\n"
               "  - Left Click: Single shot\n"
               "  - Right Click: Burst fire\n\n"
               "GUN 2 (Press 2): Explosive rounds\n"
               "  - Left Click: Explosive bubble\n"
               "  - Right Click: Deploy mine\n\n"
               "Tip: Shoot mines with explosive rounds for chain reactions!";
        text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(
            ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - text_size.y / 2 + 50.f));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);
        ImGui::PopFont();
    }
}

void TutorialUI::draw_tutorial_upgrade_system(TutorialStep tutorial) {
    (void)tutorial;
    ImGuiIO &io = ImGui::GetIO();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 rect_min_filled = ImVec2(pos.x, pos.y);
    ImVec2 rect_max_filled = ImVec2(rect_min_filled.x + size.x, rect_min_filled.y + size.y);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, IM_COL32(10, 10, 10, 180), 0.f, 0.f);

    ImGui::PushFont(io.Fonts->Fonts[1]);
    const char *text = "UPGRADE SYSTEM";
    ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - 190.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[2]);
    text = "After each level, you'll get an upgrade screen.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 - 40.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);

    text = "Defeat enemies to earn crackers, then spend them on Attack, Max Ammo, or Max HP.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 + 30.f));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", text);

    text = "Your tutorial upgrades won't carry into Level 1.";
    text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(WINDOW_WIDTH_PX / 2 - text_size.x / 2, WINDOW_HEIGHT_PX / 2 + 100.f));
    ImGui::TextColored(ImVec4(0.75f, 0.95f, 0.75f, 1.f), "%s", text);
    ImGui::PopFont();
}

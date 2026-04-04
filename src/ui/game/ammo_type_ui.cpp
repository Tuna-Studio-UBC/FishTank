#include "game_ui.hpp"

#include <cmath>

namespace {
struct SlotStyle {
    ImU32 fill;
    ImU32 shade;
    ImU32 border;
    ImU32 accent;
};

SlotStyle get_slot_style(bool selected) {
    if (selected) {
        return {
            IM_COL32(98, 108, 96, 228),
            IM_COL32(63, 73, 62, 228),
            IM_COL32(244, 232, 176, 255),
            IM_COL32(255, 211, 84, 255),
        };
    }

    return {
        IM_COL32(54, 58, 55, 210),
        IM_COL32(34, 38, 36, 210),
        IM_COL32(113, 122, 116, 240),
        IM_COL32(173, 180, 175, 230),
    };
}

void draw_bounce_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    dl->AddCircle(center, 5.f * scale, colour, 0, 2.f);
    dl->AddCircleFilled(ImVec2(center.x - 1.8f * scale, center.y - 1.4f * scale), 2.5f * scale, colour);
    dl->AddLine(ImVec2(center.x + 2.8f * scale, center.y + 3.2f * scale), ImVec2(center.x + 7.5f * scale, center.y + 7.5f * scale),
                colour, 1.8f);
    dl->AddLine(ImVec2(center.x + 7.5f * scale, center.y + 7.5f * scale),
                ImVec2(center.x + 4.8f * scale, center.y + 7.5f * scale), colour, 1.8f);
    dl->AddLine(ImVec2(center.x + 7.5f * scale, center.y + 7.5f * scale),
                ImVec2(center.x + 7.5f * scale, center.y + 4.8f * scale), colour, 1.8f);
}

void draw_burst_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    for (int i = 0; i < 3; ++i) {
        dl->AddCircleFilled(ImVec2(center.x + (i - 1) * 4.8f * scale, center.y), 2.f * scale, colour);
    }
}

void draw_explosive_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    dl->AddCircleFilled(center, 4.5f * scale, colour);
    dl->AddLine(ImVec2(center.x, center.y - 5.5f * scale), ImVec2(center.x + 3.6f * scale, center.y - 10.f * scale), colour, 1.6f);
    dl->AddCircleFilled(ImVec2(center.x + 5.f * scale, center.y - 11.4f * scale), 1.4f * scale, colour);
    dl->AddLine(ImVec2(center.x - 8.5f * scale, center.y), ImVec2(center.x - 4.8f * scale, center.y), colour, 1.6f);
    dl->AddLine(ImVec2(center.x + 4.8f * scale, center.y), ImVec2(center.x + 8.5f * scale, center.y), colour, 1.6f);
    dl->AddLine(ImVec2(center.x, center.y + 4.8f * scale), ImVec2(center.x, center.y + 8.5f * scale), colour, 1.6f);
}

void draw_mine_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    dl->AddCircle(center, 5.f * scale, colour, 0, 2.f);
    dl->AddCircleFilled(center, 2.4f * scale, colour);
    for (int i = 0; i < 8; ++i) {
        float angle = (2.f * M_PI * i) / 8.f;
        ImVec2 inner(center.x + std::cos(angle) * 5.f * scale, center.y + std::sin(angle) * 5.f * scale);
        ImVec2 outer(center.x + std::cos(angle) * 8.5f * scale, center.y + std::sin(angle) * 8.5f * scale);
        dl->AddLine(inner, outer, colour, 1.6f);
    }
}

void draw_minigun_single_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    const ImVec2 muzzle(center.x + 7.0f * scale, center.y);
    dl->AddLine(ImVec2(center.x - 8.0f * scale, center.y), muzzle, colour, 2.2f);
    for (int i = -1; i <= 1; ++i) {
        const float offset = i * 2.6f * scale;
        dl->AddLine(ImVec2(center.x - 4.5f * scale, center.y + offset), ImVec2(center.x + 4.2f * scale, center.y + offset),
                    colour, 1.4f);
    }
    dl->AddTriangleFilled(ImVec2(muzzle.x + 4.0f * scale, muzzle.y), ImVec2(muzzle.x, muzzle.y - 2.4f * scale),
                          ImVec2(muzzle.x, muzzle.y + 2.4f * scale), colour);
}

void draw_minigun_barrage_icon(ImDrawList *dl, const ImVec2 &center, float scale, ImU32 colour) {
    for (int i = -1; i <= 1; ++i) {
        const float y_offset = i * 3.2f * scale;
        dl->AddLine(ImVec2(center.x - 7.5f * scale, center.y + y_offset),
                    ImVec2(center.x + 4.8f * scale, center.y + y_offset * 0.55f), colour, 1.6f);
        dl->AddTriangleFilled(ImVec2(center.x + 7.5f * scale, center.y + y_offset * 0.45f),
                              ImVec2(center.x + 3.5f * scale, center.y - 1.8f * scale + y_offset * 0.45f),
                              ImVec2(center.x + 3.5f * scale, center.y + 1.8f * scale + y_offset * 0.45f), colour);
    }
}

void draw_ammo_icon(ImDrawList *dl, WeaponType weapon, bool left_click, const ImVec2 &center, float scale, ImU32 colour) {
    if (weapon == WeaponType::GUN_1 && left_click) {
        draw_bounce_icon(dl, center, scale, colour);
    } else if (weapon == WeaponType::GUN_1) {
        draw_burst_icon(dl, center, scale, colour);
    } else if (weapon == WeaponType::GUN_2 && left_click) {
        draw_explosive_icon(dl, center, scale, colour);
    } else if (weapon == WeaponType::GUN_2) {
        draw_mine_icon(dl, center, scale, colour);
    } else if (left_click) {
        draw_minigun_single_icon(dl, center, scale, colour);
    } else {
        draw_minigun_barrage_icon(dl, center, scale, colour);
    }
}

const char *get_ammo_cost_text(WeaponType weapon, bool left_click) {
    if (weapon == WeaponType::GUN_1) {
        return left_click ? "1" : "3";
    }

    if (weapon == WeaponType::GUN_2) {
        return left_click ? "10" : "5";
    }

    return left_click ? "1" : "ALL";
}

const char *get_weapon_label(WeaponType weapon) {
    if (weapon == WeaponType::GUN_1) {
        return "1";
    }
    if (weapon == WeaponType::GUN_2) {
        return "2";
    }
    return "3";
}

void draw_slot(ImDrawList *dl, const ImVec2 &min, const ImVec2 &size, WeaponType weapon, bool selected) {
    SlotStyle style = get_slot_style(selected);
    ImVec2 max(min.x + size.x, min.y + size.y);

    dl->AddRectFilled(min, max, style.fill, 1.5f);
    dl->AddRectFilled(ImVec2(min.x, min.y + size.y * 0.55f), max, style.shade, 1.5f);
    dl->AddRect(min, max, IM_COL32(18, 22, 20, 220), 1.5f, 0, 2.f);
    dl->AddRect(ImVec2(min.x - 1.5f, min.y - 1.5f), ImVec2(max.x + 1.5f, max.y + 1.5f), style.border, 2.f, 0,
                selected ? 3.f : 2.f);

    dl->AddText(ImVec2(min.x + 6.f, min.y + 3.f), selected ? IM_COL32(255, 245, 210, 235) : IM_COL32(214, 214, 214, 190),
                get_weapon_label(weapon));

    const float lane_top = min.y + 5.f;
    const float lane_height = 11.f;
    const float lane_gap = 3.f;
    const float lane_inset_left = 16.f;
    const float lane_inset_right = 4.f;
    const float icon_x = min.x + 28.f;

    for (int i = 0; i < 2; ++i) {
        bool left_click = (i == 0);
        float y = lane_top + i * (lane_height + lane_gap);
        ImVec2 lane_min(min.x + lane_inset_left, y);
        ImVec2 lane_max(max.x - lane_inset_right, y + lane_height);
        dl->AddRectFilled(lane_min, lane_max, IM_COL32(0, 0, 0, selected ? 40 : 55), 1.5f);
        dl->AddRect(lane_min, lane_max, IM_COL32(255, 255, 255, selected ? 32 : 20), 1.5f);
        draw_ammo_icon(dl, weapon, left_click, ImVec2(icon_x, y + lane_height * 0.5f), 0.72f, style.accent);

        const std::string cost_text = get_ammo_cost_text(weapon, left_click);
        ImVec2 cost_size = ImGui::CalcTextSize(cost_text.c_str());
        dl->AddText(ImVec2(lane_max.x - cost_size.x - 3.f, y + (lane_height - cost_size.y) * 0.5f - 1.f),
                    selected ? IM_COL32(255, 245, 210, 235) : IM_COL32(214, 214, 214, 190), cost_text.c_str());
    }
}
} // namespace

void GameUI::draw_ammo_types() {
    if (registry.players.entities.empty())
        return;

    Entity player = registry.players.entities[0];
    if (!registry.weaponStates.has(player))
        return;

    WeaponState &ws = registry.weaponStates.get(player);

    const float slot_width = 72.f;
    const float slot_height = 36.f;
    const float gap = 4.f;
    const ImVec2 total_size(slot_width * 3.f + gap * 2.f, slot_height);

    ImGui::SetNextWindowSize(total_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * 0.5f, WINDOW_HEIGHT_PX - 18.f), ImGuiCond_Always, ImVec2(0.5f, 1.f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

    if (!ImGui::Begin("Ammo Types", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();

    draw_slot(dl, pos, ImVec2(slot_width, slot_height), WeaponType::GUN_1, ws.current_weapon == WeaponType::GUN_1);
    draw_slot(dl, ImVec2(pos.x + slot_width + gap, pos.y), ImVec2(slot_width, slot_height), WeaponType::GUN_2, ws.current_weapon == WeaponType::GUN_2);
    draw_slot(dl, ImVec2(pos.x + (slot_width + gap) * 2.f, pos.y), ImVec2(slot_width, slot_height), WeaponType::GUN_3,
              ws.current_weapon == WeaponType::GUN_3);

    ImGui::Dummy(total_size);
    ImGui::End();
}

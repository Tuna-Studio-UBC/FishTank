#include "common.hpp"
#include "game_ui.hpp"
#include "imgui.h"
#include "tinyECS/registry.hpp"
#include <algorithm>

static ImVec2 world_to_minimap(const vec2 &world_pos, const ImVec2 &map_min, const ImVec2 &map_size, float world_w,
                               float world_h) {
    float nx = (world_w > 0.f) ? (world_pos.x / world_w) : 0.f;
    float ny = (world_h > 0.f) ? (world_pos.y / world_h) : 0.f;
    nx = std::clamp(nx, 0.f, 1.f);
    ny = std::clamp(ny, 0.f, 1.f);
    return ImVec2(map_min.x + nx * map_size.x, map_min.y + ny * map_size.y);
}

static ImU32 powerup_color(POWER_UP_TYPE t) {
    switch (t) {
    case POWER_UP_TYPE::HEAL:
        return IM_COL32(50, 255, 50, 240);
    case POWER_UP_TYPE::SPEED_MULTIPLIER:
        return IM_COL32(80, 255, 255, 240);
    case POWER_UP_TYPE::DAMAGE_MULTIPLIER:
        return IM_COL32(255, 255, 80, 240);
    default:
        return IM_COL32(200, 200, 200, 220);
    }
}

static ImU32 hazard_color(HAZARD_TYPE t) {
    switch (t) {
    case HAZARD_TYPE::MINE:
        return IM_COL32(255, 120, 60, 240);
    case HAZARD_TYPE::ANEMONE:
        return IM_COL32(255, 60, 160, 240);
    default:
        return IM_COL32(255, 165, 0, 230);
    }
}

void GameUI::draw_minimap() {
    if (registry.tilemaps.components.empty())
        return;

    const TileMap &tilemap = registry.tilemaps.components[0];
    const float world_w = tilemap.width * (float)TILE_SIZE_PX;
    const float world_h = tilemap.height * (float)TILE_SIZE_PX;

    ImGuiIO &io = ImGui::GetIO();
    const float padding = 12.f;
    const ImVec2 win_size(240.f, 240.f);

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - win_size.x - padding, padding), ImGuiCond_Always);
    ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin("Minimap", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float side = std::min(avail.x, avail.y);
    ImVec2 map_size(side, side);
    ImVec2 map_min = p0;
    ImVec2 map_max(map_min.x + map_size.x, map_min.y + map_size.y);
    ImDrawList *dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(map_min, map_max, IM_COL32(10, 10, 10, 150), 6.f);
    dl->AddRect(map_min, map_max, IM_COL32(255, 255, 255, 90), 6.f, 0, 2.f);

    // wall
    for (Entity e : registry.walls.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);
        const float r = 2.0f;
        dl->AddRectFilled(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), IM_COL32(220, 220, 220, 190));
    }

    // hazard
    for (Entity e : registry.hazards.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        HAZARD_TYPE ht = registry.hazards.get(e).type;
        ImU32 col = hazard_color(ht);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);
        dl->AddCircleFilled(c, 3.2f, col);
    }

    // power-up
    for (Entity e : registry.pickUps.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);

        if (!registry.powerUps.has(e)) {
            dl->AddCircleFilled(c, 3.0f, IM_COL32(120, 255, 120, 220));
            continue;
        }

        POWER_UP_TYPE pt = registry.powerUps.get(e).type;
        ImU32 col = powerup_color(pt);

        if (pt == POWER_UP_TYPE::SPEED_MULTIPLIER) {
            ImVec2 a(c.x, c.y - 4.f), b(c.x - 3.5f, c.y + 3.0f), d(c.x + 3.5f, c.y + 3.0f);
            dl->AddTriangleFilled(a, b, d, col);
        } else if (pt == POWER_UP_TYPE::DAMAGE_MULTIPLIER) {
            ImVec2 top(c.x, c.y - 4.f), left(c.x - 4.f, c.y), bot(c.x, c.y + 4.f), right(c.x + 4.f, c.y);
            dl->AddQuadFilled(top, left, bot, right, col);
        } else {
            dl->AddCircleFilled(c, 3.5f, col);
        }
    }

    // bubbler
    for (Entity e : registry.bubblers.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);
        dl->AddCircleFilled(c, 4.2f, IM_COL32(212, 122, 80, 255));
        dl->AddCircle(c, 4.2f, IM_COL32(255, 255, 255, 170), 0, 2.f);
    }

    // enemy
    for (Entity e : registry.enemies.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);
        dl->AddCircleFilled(c, 3.2f, IM_COL32(255, 70, 70, 235));
    }

    // player
    for (Entity e : registry.players.entities) {
        if (!registry.motions.has(e))
            continue;
        const Motion &m = registry.motions.get(e);

        ImVec2 c = world_to_minimap(m.position, map_min, map_size, world_w, world_h);
        dl->AddCircleFilled(c, 4.2f, IM_COL32(80, 170, 255, 255));
        dl->AddCircle(c, 4.2f, IM_COL32(255, 255, 255, 170), 0, 2.f);
    }

    ImGui::Dummy(map_size);
    ImGui::End();
}

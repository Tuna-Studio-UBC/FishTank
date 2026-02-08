#include "game_ui.hpp"

void GameUI::draw_healthbar()
{
  float bar_width = 300;
  float bar_height = 10;
  float padding = 2;
  ImGui::SetNextWindowSize(ImVec2(bar_width, bar_height));
  ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH_PX * .5f, WINDOW_HEIGHT_PX * .9f), ImGuiCond_Once, ImVec2(.5f, .5f));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.2f, .2f, .2f, .9f));
  ImGui::Begin("Health Bar", nullptr, window_flags);

  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 pos = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();

  ImU32 colour_start = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
  ImU32 colour_end = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 1.f, 0.f, 1.f));

  Entity e = registry.players.entities[0];

  if (registry.health.has(e))
  {
    Health &h = registry.health.get(e);
    float ratio = (h.max_hp > 0) ? (float)h.hp / (float)h.max_hp : 0.f;
    ImVec2 rect_min_filled = ImVec2(pos.x + padding, pos.y + padding);
    ImVec2 rect_max_filled = ImVec2(pos.x + (size.x - padding) * ratio, pos.y + (size.y - padding));
    ImU32 colour = colour_lerp(colour_start, colour_end, ratio);
    draw_list->AddRectFilled(rect_min_filled, rect_max_filled, colour);

    ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), std::string("HP: " + std::to_string(h.hp) + "/" + std::to_string(h.max_hp)).c_str());
  }

  ImGui::End();
  ImGui::PopStyleColor();
}

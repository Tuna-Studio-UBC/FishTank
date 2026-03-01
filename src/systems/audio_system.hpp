#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class AudioSystem {
public:
    bool load();
    void shutdown();

    void update_hazard_warning(const vec2& listener_pos, float elapsed_ms);
    void play_enemy_shot(const vec2& source_pos, const vec2& listener_pos);
    void play_player_shot_center(int volume);
    void play_bgm(const std::string& filename, int volume);

private:
    void apply_spatial_to_channel(int channel, const vec2& source_pos, const vec2& listener_pos, float max_dist_px, float pan_dist_px, int base_volume);

    Mix_Chunk* hazard_warn_loop = nullptr;
    Mix_Chunk* enemy_shot = nullptr;

    Mix_Chunk* player_shot = nullptr;
    Mix_Music* bgm = nullptr;

    int hazard_warn_channel = -1;
    Entity current_hazard;
    float hazard_update_acc_ms = 0.f;
};
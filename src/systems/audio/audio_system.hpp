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

    void update_hazard_warning(const vec2 &listener_pos, float elapsed_ms);
    void play_enemy_shot(const vec2 &source_pos, const vec2 &listener_pos);
    void play_player_shot_center(int volume);
    void play_cannon_shot_center(int volume = 110);
    void play_shot_mine_center(int volume = 100);
    void play_power_up_center(int volume = 96);
    void play_hit_hazard_center(int volume = 104);
    void play_enemy_dead_center(int volume = 100);
    void play_complete_level_center(int volume = 110);
    void play_upgrade_center(int volume = 100);
    void play_ammo_load_center(int volume = 100);
    void play_clear_game_center(int volume = 112);
    void play_game_over_center(int volume = 108);
    void play_no_ammo_center(int volume = 96);
    void play_bgm(const std::string &filename, int volume);
    void set_master_volume(int volume);
    int get_master_volume() const;

  private:
    void apply_spatial_to_channel(int channel, const vec2 &source_pos, const vec2 &listener_pos, float max_dist_px,
                                  float pan_dist_px, int base_volume);
    void apply_master_volume() const;

    Mix_Chunk *hazard_warn_loop = nullptr;
    Mix_Chunk *enemy_shot = nullptr;
    Mix_Chunk *player_shot = nullptr;
    Mix_Chunk *cannon_shot = nullptr;
    Mix_Chunk *shot_mine = nullptr;
    Mix_Chunk *power_up = nullptr;
    Mix_Chunk *hit_hazard = nullptr;
    Mix_Chunk *enemy_dead = nullptr;
    Mix_Chunk *complete_level = nullptr;
    Mix_Chunk *upgrade = nullptr;
    Mix_Chunk *ammo_load = nullptr;
    Mix_Chunk *clear_game = nullptr;
    Mix_Chunk *game_over = nullptr;
    Mix_Chunk *no_ammo = nullptr;
    Mix_Music *bgm = nullptr;

    int hazard_warn_channel = -1;
    int bgm_base_volume = 45;
    int master_volume = MIX_MAX_VOLUME;
    Entity current_hazard;
    float hazard_update_acc_ms = 0.f;
};

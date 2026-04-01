#include "systems/audio/audio_system.hpp"
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>
#include <unordered_set>

static int clamp_int(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }

void AudioSystem::apply_master_volume() const {
    Mix_MasterVolume(master_volume);
    const int music_volume = (bgm_base_volume * master_volume) / MIX_MAX_VOLUME;
    Mix_VolumeMusic(clamp_int(music_volume, 0, MIX_MAX_VOLUME));
}

bool AudioSystem::load() {
    hazard_warn_loop = Mix_LoadWAV(get_path(AUDIO, "hazard_warn_loop.wav").c_str());
    enemy_shot = Mix_LoadWAV(get_path(AUDIO, "enemy_shot.wav").c_str());
    Mix_VolumeChunk(enemy_shot, 96);
    player_shot = Mix_LoadWAV(get_path(AUDIO, "player_shot.wav").c_str());
    cannon_shot = Mix_LoadWAV(get_path(AUDIO, "canon_shot.wav").c_str());
    Mix_VolumeChunk(cannon_shot, 104);
    shot_mine = Mix_LoadWAV(get_path(AUDIO, "shot_mine.wav").c_str());
    Mix_VolumeChunk(shot_mine, 96);
    power_up = Mix_LoadWAV(get_path(AUDIO, "power_up.wav").c_str());
    Mix_VolumeChunk(power_up, 96);
    hit_hazard = Mix_LoadWAV(get_path(AUDIO, "hit_hazard.wav").c_str());
    Mix_VolumeChunk(hit_hazard, 104);
    enemy_dead = Mix_LoadWAV(get_path(AUDIO, "enemy_dead.wav").c_str());
    Mix_VolumeChunk(enemy_dead, 96);
    complete_level = Mix_LoadWAV(get_path(AUDIO, "complete_level.wav").c_str());
    Mix_VolumeChunk(complete_level, 108);
    upgrade = Mix_LoadWAV(get_path(AUDIO, "upgrade.wav").c_str());
    Mix_VolumeChunk(upgrade, 96);
    ammo_load = Mix_LoadWAV(get_path(AUDIO, "ammo_load.wav").c_str());
    Mix_VolumeChunk(ammo_load, 96);
    clear_game = Mix_LoadWAV(get_path(AUDIO, "clear_game.wav").c_str());
    Mix_VolumeChunk(clear_game, 108);
    game_over = Mix_LoadWAV(get_path(AUDIO, "game_over.wav").c_str());
    Mix_VolumeChunk(game_over, 108);
    no_ammo = Mix_LoadWAV(get_path(AUDIO, "no_ammo.wav").c_str());
    Mix_VolumeChunk(no_ammo, 96);

    if (!hazard_warn_loop || !enemy_shot || !player_shot || !cannon_shot || !shot_mine || !power_up || !hit_hazard ||
        !enemy_dead || !complete_level || !upgrade || !ammo_load || !clear_game || !game_over || !no_ammo) {
        spdlog::error("[Audio] Failed to load audio. Ensure files exist in res/audio/");
        spdlog::error("[Audio] hazard_warn_loop.wav path = {}", get_path(AUDIO, "hazard_warn_loop.wav"));
        spdlog::error("[Audio] enemy_shot.wav path = {}", get_path(AUDIO, "enemy_shot.wav"));
        spdlog::error("[Audio] player_shot.wav path = {}", get_path(AUDIO, "player_shot.wav"));
        spdlog::error("[Audio] canon_shot.wav path = {}", get_path(AUDIO, "canon_shot.wav"));
        spdlog::error("[Audio] shot_mine.wav path = {}", get_path(AUDIO, "shot_mine.wav"));
        spdlog::error("[Audio] power_up.wav path = {}", get_path(AUDIO, "power_up.wav"));
        spdlog::error("[Audio] hit_hazard.wav path = {}", get_path(AUDIO, "hit_hazard.wav"));
        spdlog::error("[Audio] enemy_dead.wav path = {}", get_path(AUDIO, "enemy_dead.wav"));
        spdlog::error("[Audio] complete_level.wav path = {}", get_path(AUDIO, "complete_level.wav"));
        spdlog::error("[Audio] upgrade.wav path = {}", get_path(AUDIO, "upgrade.wav"));
        spdlog::error("[Audio] ammo_load.wav path = {}", get_path(AUDIO, "ammo_load.wav"));
        spdlog::error("[Audio] clear_game.wav path = {}", get_path(AUDIO, "clear_game.wav"));
        spdlog::error("[Audio] game_over.wav path = {}", get_path(AUDIO, "game_over.wav"));
        spdlog::error("[Audio] no_ammo.wav path = {}", get_path(AUDIO, "no_ammo.wav"));
        return false;
    }

    Mix_AllocateChannels(32);
    apply_master_volume();
    spdlog::info("[Audio] Loaded SFX");
    return true;
}

void AudioSystem::shutdown() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    if (hazard_warn_channel != -1) {
        Mix_HaltChannel(hazard_warn_channel);
        hazard_warn_channel = -1;
    }

    Mix_HaltChannel(-1);

    if (hazard_warn_loop) {
        Mix_FreeChunk(hazard_warn_loop);
        hazard_warn_loop = nullptr;
    }
    if (enemy_shot) {
        Mix_FreeChunk(enemy_shot);
        enemy_shot = nullptr;
    }

    if (player_shot) {
        Mix_FreeChunk(player_shot);
        player_shot = nullptr;
    }
    if (cannon_shot) {
        Mix_FreeChunk(cannon_shot);
        cannon_shot = nullptr;
    }
    if (shot_mine) {
        Mix_FreeChunk(shot_mine);
        shot_mine = nullptr;
    }
    if (power_up) {
        Mix_FreeChunk(power_up);
        power_up = nullptr;
    }
    if (hit_hazard) {
        Mix_FreeChunk(hit_hazard);
        hit_hazard = nullptr;
    }
    if (enemy_dead) {
        Mix_FreeChunk(enemy_dead);
        enemy_dead = nullptr;
    }
    if (complete_level) {
        Mix_FreeChunk(complete_level);
        complete_level = nullptr;
    }
    if (upgrade) {
        Mix_FreeChunk(upgrade);
        upgrade = nullptr;
    }
    if (ammo_load) {
        Mix_FreeChunk(ammo_load);
        ammo_load = nullptr;
    }
    if (clear_game) {
        Mix_FreeChunk(clear_game);
        clear_game = nullptr;
    }
    if (game_over) {
        Mix_FreeChunk(game_over);
        game_over = nullptr;
    }
    if (no_ammo) {
        Mix_FreeChunk(no_ammo);
        no_ammo = nullptr;
    }

    if (bgm) {
        Mix_FreeMusic(bgm);
        bgm = nullptr;
    }
}

void AudioSystem::apply_spatial_to_channel(int channel, const vec2 &source_pos, const vec2 &listener_pos,
                                           float max_dist_px, float pan_dist_px, int base_volume) {
    vec2 d = source_pos - listener_pos;
    float dist = length(d);
    float gain = 1.f - (dist / max_dist_px);
    gain = std::clamp(gain, 0.f, 1.f);

    int vol = (int)std::round(base_volume * gain);
    vol = clamp_int(vol, 0, 128);
    Mix_Volume(channel, vol);

    float pan = 0.f;
    if (pan_dist_px > 1e-3f) {
        pan = std::clamp(d.x / pan_dist_px, -1.f, 1.f);
    }

    float left_f = (1.f - pan) * 0.5f;
    float right_f = (1.f + pan) * 0.5f;

    int left = clamp_int((int)std::round(left_f * 255.f), 0, 255);
    int right = clamp_int((int)std::round(right_f * 255.f), 0, 255);

    Mix_SetPanning(channel, (Uint8)left, (Uint8)right);
}

void AudioSystem::update_hazard_warning(const vec2 &listener_pos, float elapsed_ms) {
    if (!hazard_warn_loop)
        return;

    hazard_update_acc_ms += elapsed_ms;
    const float UPDATE_MS = 100.f;
    if (hazard_update_acc_ms < UPDATE_MS)
        return;
    hazard_update_acc_ms = 0.f;

    const float R_WARN = 3.f * TILE_SIZE_PX;
    const float PAN_DIST = 4.f * TILE_SIZE_PX;
    const int BASE_VOL = 96;

    Entity best;
    float best_d2 = 1e30f;

    for (Entity h : registry.hazards.entities) {
        if (!registry.motions.has(h))
            continue;
        vec2 hp = registry.motions.get(h).position;
        vec2 diff = hp - listener_pos;
        float d2 = dot(diff, diff);
        if (d2 < best_d2) {
            best_d2 = d2;
            best = h;
        }
    }

    if (best_d2 > 9e29f) {
        if (hazard_warn_channel != -1) {
            Mix_HaltChannel(hazard_warn_channel);
            hazard_warn_channel = -1;
        }
        return;
    }

    float best_d = std::sqrt(best_d2);

    if (best_d <= R_WARN) {
        if (hazard_warn_channel == -1) {
            hazard_warn_channel = Mix_PlayChannel(-1, hazard_warn_loop, -1);
            current_hazard = best;
        }

        if (hazard_warn_channel != -1 && registry.motions.has(best)) {
            vec2 src = registry.motions.get(best).position;
            apply_spatial_to_channel(hazard_warn_channel, src, listener_pos, R_WARN, PAN_DIST, BASE_VOL);
        }
    } else {
        if (hazard_warn_channel != -1) {
            Mix_HaltChannel(hazard_warn_channel);
            hazard_warn_channel = -1;
        }
    }
}

void AudioSystem::play_enemy_shot(const vec2 &source_pos, const vec2 &listener_pos) {
    if (!enemy_shot)
        return;

    const float MAX_DIST = 12.f * TILE_SIZE_PX;
    const float PAN_DIST = 5.f * TILE_SIZE_PX;
    const int BASE_VOL = 110;

    int ch = Mix_PlayChannel(-1, enemy_shot, 0);
    if (ch == -1)
        return;

    apply_spatial_to_channel(ch, source_pos, listener_pos, MAX_DIST, PAN_DIST, BASE_VOL);
}

void AudioSystem::play_player_shot_center(int volume) {
    if (!player_shot)
        return;
    int ch = Mix_PlayChannel(-1, player_shot, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_cannon_shot_center(int volume) {
    if (!cannon_shot)
        return;
    int ch = Mix_PlayChannel(-1, cannon_shot, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_shot_mine_center(int volume) {
    if (!shot_mine)
        return;
    int ch = Mix_PlayChannel(-1, shot_mine, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_power_up_center(int volume) {
    if (!power_up)
        return;
    int ch = Mix_PlayChannel(-1, power_up, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_hit_hazard_center(int volume) {
    if (!hit_hazard)
        return;
    int ch = Mix_PlayChannel(-1, hit_hazard, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_enemy_dead_center(int volume) {
    if (!enemy_dead)
        return;
    int ch = Mix_PlayChannel(-1, enemy_dead, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_complete_level_center(int volume) {
    if (!complete_level)
        return;
    int ch = Mix_PlayChannel(-1, complete_level, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_upgrade_center(int volume) {
    if (!upgrade)
        return;
    int ch = Mix_PlayChannel(-1, upgrade, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_ammo_load_center(int volume) {
    if (!ammo_load)
        return;
    int ch = Mix_PlayChannel(-1, ammo_load, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_clear_game_center(int volume) {
    if (!clear_game)
        return;
    int ch = Mix_PlayChannel(-1, clear_game, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_game_over_center(int volume) {
    if (!game_over)
        return;
    int ch = Mix_PlayChannel(-1, game_over, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_no_ammo_center(int volume) {
    if (!no_ammo)
        return;
    int ch = Mix_PlayChannel(-1, no_ammo, 0);
    if (ch == -1)
        return;
    Mix_Volume(ch, std::clamp(volume, 0, 128));
    Mix_SetPanning(ch, 255, 255);
}

void AudioSystem::play_bgm(const std::string &filename, int volume) {
    bgm_base_volume = clamp_int(volume, 0, MIX_MAX_VOLUME);

    if (bgm) {
        Mix_FreeMusic(bgm);
        bgm = nullptr;
    }

    bgm = Mix_LoadMUS(get_path(AUDIO, filename).c_str());

    if (!bgm) {
        printf("Failed to load BGM: %s\n", filename.c_str());
        return;
    }

    apply_master_volume();
    Mix_PlayMusic(bgm, -1);
}

void AudioSystem::set_master_volume(int volume) {
    master_volume = clamp_int(volume, 0, MIX_MAX_VOLUME);
    apply_master_volume();
}

int AudioSystem::get_master_volume() const { return master_volume; }

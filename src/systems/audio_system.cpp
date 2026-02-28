#include "systems/audio_system.hpp"
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>
#include <unordered_set>

static int clamp_int(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }

bool AudioSystem::load() {
    hazard_warn_loop = Mix_LoadWAV(get_path(AUDIO, "hazard_warn_loop.wav").c_str());
    enemy_shot = Mix_LoadWAV(get_path(AUDIO, "enemy_shot.wav").c_str());

    if (!hazard_warn_loop || !enemy_shot) {
        spdlog::error("[Audio] Failed to load audio. Ensure files exist in res/audio/");
        spdlog::error("[Audio] hazard_warn_loop.wav path = {}", get_path(AUDIO, "hazard_warn_loop.wav"));
        spdlog::error("[Audio] enemy_shot.wav path = {}", get_path(AUDIO, "enemy_shot.wav"));
        return false;
    }

    Mix_AllocateChannels(32);
    spdlog::info("[Audio] Loaded SFX");
    return true;
}

void AudioSystem::shutdown() {
    if (hazard_warn_loop) {
        Mix_FreeChunk(hazard_warn_loop);
        hazard_warn_loop = nullptr;
    }
    if (enemy_shot) {
        Mix_FreeChunk(enemy_shot);
        enemy_shot = nullptr;
    }

    hazard_warn_channel = -1;
}

void AudioSystem::apply_spatial_to_channel(int channel, const vec2& source_pos, const vec2& listener_pos, float max_dist_px, float pan_dist_px, int base_volume) {
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

void AudioSystem::update_hazard_warning(const vec2& listener_pos, float elapsed_ms) {
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

    for (Entity h : registry.hazard.entities) {
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

void AudioSystem::play_enemy_shot(const vec2& source_pos, const vec2& listener_pos) {
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
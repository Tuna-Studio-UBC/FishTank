#include "pick_up_system.hpp"

#include <spdlog/spdlog.h>

void PickUpSystem::set_audio_system(AudioSystem *audio_system) { this->audio_system = audio_system; }

void PickUpSystem::step(float elapsed_ms) {
    for (Entity e : registry.pickUps.entities) {
        Entity player = registry.players.entities[0];
        vec2 player_pos = registry.motions.get(player).position;

        if (registry.motions.has(e)) {
            Motion &motion = registry.motions.get(e);
            if (entity_in_radius(player_pos - motion.position, motion.scale, 0.f)) {
                on_pickup(e, player);
            }
        }
    }
}

void PickUpSystem::on_pickup(Entity pickUp, Entity player) {
    bool picked_up = false;

    if (registry.powerUps.has(pickUp)) {
        PowerUp powerUp = registry.powerUps.get(pickUp);
        auto &config = POWER_UP_CONFIGS[(int)powerUp.type];

        spdlog::debug("Picked up {} of type: {}", pickUp.name_id(), (int)powerUp.type);

        // TODO: Add more power ups!!!
        switch (powerUp.type) {
        case POWER_UP_TYPE::HEAL:

            // Skip if player is already at full health
            if (registry.healths.get(player).hp == registry.healths.get(player).max_hp)
                return;

            createOverTimeEffect<Heal>(player, config.duration_ms, config.interval_ms, true, config.amount);
            picked_up = true;
            break;
        case POWER_UP_TYPE::SPEED_MULTIPLIER:
            createExpirationEffect<SpeedMultiplier>(player, config.duration_ms, config.amount);
            picked_up = true;
            break;
        case POWER_UP_TYPE::DAMAGE_MULTIPLIER:
            createExpirationEffect<DamageMultiplier>(player, config.duration_ms, config.amount);
            picked_up = true;
            break;
        default:
            spdlog::warn("Unknown power up type");
            break;
        }
    }

    if (picked_up && audio_system != nullptr) {
        audio_system->play_power_up_center();
    }

    registry.remove_all_components_of(pickUp);
}

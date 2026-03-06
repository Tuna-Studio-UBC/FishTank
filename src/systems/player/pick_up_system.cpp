#include "pick_up_system.hpp"

#include <spdlog/spdlog.h>

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
            break;
        case POWER_UP_TYPE::SPEED_MULTIPLIER:
            createExpirationEffect<SpeedMultiplier>(player, config.duration_ms, config.amount);
            break;
        case POWER_UP_TYPE::DAMAGE_MULTIPLIER:
            createExpirationEffect<DamageMultiplier>(player, config.duration_ms, config.amount);
            break;
        default:
            spdlog::warn("Unknown power up type");
            break;
        }
    }

    registry.remove_all_components_of(pickUp);
}

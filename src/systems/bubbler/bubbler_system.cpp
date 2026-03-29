#include "bubbler_system.hpp"

void BubblerSystem::step(float elapsed_ms) {
    for (Entity &bubbler_entity : registry.bubblers.entities) {
        Bubbler& bubbler = registry.bubblers.get(bubbler_entity);

        if (bubbler.charged)
            continue;

        vec2& bubbler_pos = registry.motions.get(bubbler_entity).position;
        bool player_in_range = false;

        for (Entity &player : registry.players.entities) {
            if (registry.motions.has(player)) {
                vec2 &player_pos = registry.motions.get(player).position;
                if (glm::distance(bubbler_pos, player_pos) < DISTANCE_THRESHOLD) {
                    player_in_range = true;
                    break;
                }
            }
        }

        if (!player_in_range) {
            bubbler.recharge_timer += elapsed_ms;
            if (bubbler.recharge_timer >= CHARGE_TIME)
                bubbler.charged = true;
        }
    }
}
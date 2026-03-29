#include "bubbler_system.hpp"

void BubblerSystem::step(float elapsed_ms) {
    handle_bubblers(elapsed_ms);
    handle_bubbler_bubbles();
}

void BubblerSystem::handle_bubblers(float& elapsed_ms) {
    for (Entity &bubbler_entity : registry.bubblers.entities) {
        Bubbler &bubbler = registry.bubblers.get(bubbler_entity);

        if (bubbler.charged)
            continue;

        vec2 &bubbler_pos = registry.motions.get(bubbler_entity).position;
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

void BubblerSystem::handle_bubbler_bubbles() {
    for (Entity &entity : registry.bubblerBubbles.entities) {
        Entity &parent = registry.bubblerBubbles.get(entity).parent_bubbler;
        Bubbler &bubbler = registry.bubblers.get(parent);
        Tint &tint = registry.tints.get(entity);
        Motion &motion = registry.motions.get(entity);

        float scale_factor = max(0.f, (bubbler.recharge_timer - 2000.f) / CHARGE_TIME * TILE_SIZE_PX * 0.7f);
        motion.scale.x = scale_factor;
        motion.scale.y = scale_factor;

        if (bubbler.charged)
            tint.tint = vec3(1.f);
        else
            tint.tint = vec3(0.3f);
    }
}
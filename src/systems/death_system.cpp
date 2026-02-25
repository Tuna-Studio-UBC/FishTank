#include <iostream>

#include "death_system.hpp"
#include <spdlog/spdlog.h>
#include <systems/world_system.hpp>

void DeathSystem::step(float elapsed_ms) {
    for (Entity e : registry.health.entities) {
        if (registry.dead.has(e))
            continue;

        if (registry.health.has(e)) {
            Health &h = registry.health.get(e);
            if (h.hp <= 0 && h.max_hp > 0) {

                spdlog::info("{} has died", e.name_id());
                registry.dead.emplace(e);

                if (registry.players.has(e)) {
                    // TODO: Handle player death
                    registry.progress.components[0].game_over = true;
                    Entity e = Entity("Overlay");
                    registry.overlays.emplace(e);
                    auto &motion = registry.motions.emplace(e);
                    motion.position = registry.motions.get(registry.camera.entities[0]).position;
                    motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);
                    registry.renderRequests.insert(e,
                                                   {TEXTURE_ASSET_ID::TEXTURE_LOSE_OVERLAY, EFFECT_ASSET_ID::TEXTURED,
                                                    GEOMETRY_BUFFER_ID::SPRITE, (int)RENDER_LAYER::OVERLAYS});
                } else if (registry.enemies.has(e)) {
                    // TODO: Handle enemy death

                    // Drop crackers
                    if (registry.dropsCrackers.has(e)) {
                        DropsCrackers &drops = registry.dropsCrackers.get(e);
                        Entity player = registry.players.entities[0];
                        Wallet &wallet = registry.wallet.get(player);
                        wallet.crackers += drops.cracker;

                        spdlog::info("{} collected {} crackers! Total: {}", player.name_id(), drops.cracker,
                                     wallet.crackers);
                    }

                    registry.remove_all_components_of(e);
                    registry.progress.components[0].remaining_enemies -= 1;
                }
            }
        }
    }
}

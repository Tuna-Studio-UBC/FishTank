#include <iostream>

#include "death_system.hpp"
#include "systems/world/world_init.hpp"
#include <random>
#include <spdlog/spdlog.h>
#include <systems/world/world_system.hpp>

void DeathSystem::step(float elapsed_ms) {
    for (Entity e : registry.healths.entities) {
        if (registry.deads.has(e))
            continue;

        if (registry.healths.has(e)) {
            Health &h = registry.healths.get(e);
            if (h.hp <= 0 && h.max_hp > 0) {

                spdlog::info("{} has died", e.name_id());
                registry.deads.emplace(e);

                if (registry.players.has(e)) {
                    // TODO: Handle player death
                    registry.progresses.components[0].is_game_over = true;
                    registry.progresses.components[0].has_beat_level = false;
                    Entity e = Entity("Overlay");
                    registry.overlays.emplace(e);
                    auto &motion = registry.motions.emplace(e);
                    motion.position = registry.motions.get(registry.cameras.entities[0]).position;
                    motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);
                    registry.renderRequests.insert(e,
                                                   {TEXTURE_ASSET_ID::TEXTURE_LOSE_OVERLAY, EFFECT_ASSET_ID::TEXTURED,
                                                    GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::OVERLAYS});
                } else if (registry.enemies.has(e)) {
                    // TODO: Handle enemy death

                    // Drop crackers
                    if (registry.dropsCrackers.has(e) && !registry.players.entities.empty()) {
                        static std::mt19937 rng(std::random_device{}());
                        static std::uniform_int_distribution<int> cracker_dist(1, 3);

                        DropsCrackers &drops = registry.dropsCrackers.get(e);
                        drops.cracker = cracker_dist(rng);
                        Entity player = registry.players.entities[0];
                        Wallet &wallet = registry.wallets.get(player);
                        wallet.crackers += drops.cracker;

                        spdlog::info("{} collected {} crackers! Total: {}", player.name_id(), drops.cracker,
                                     wallet.crackers);
                    }

                    Motion &m = registry.motions.get(e);
                    createEffectSmoke(m.position);

                    registry.remove_all_components_of(e);
                    registry.progresses.components[0].remaining_enemies -= 1;
                }
            }
        }
    }
}

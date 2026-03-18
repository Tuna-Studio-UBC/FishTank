#include <iostream>

#include "death_system.hpp"
#include "systems/world/world_init.hpp"
#include <spdlog/spdlog.h>
#include <systems/world/world_system.hpp>
#include <configs/enemy_config.cpp>
#include <configs/attack_pattern.cpp>

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
                    if (registry.boids.has(e)) {
                        Entity leader = registry.boids.get(e).leader;
                        Leader &ld = registry.leaders.get(leader);
                        ld.followers -= 1;
                        if (ld.followers <= 0) {
                            changeEnemyType(leader, ENEMY_TYPE::GUARD);
                        }
                    }
                    if (registry.beziers.has(e)) {
                        for (Entity follower : registry.boids.entities) {
                            if (registry.boids.get(follower).leader == e) {
                                changeEnemyType(follower, ENEMY_TYPE::HOMING);
                            }
                        }
                    }

                    // Drop crackers
                    if (registry.dropsCrackers.has(e)) {
                        DropsCrackers &drops = registry.dropsCrackers.get(e);
                        Entity player = registry.players.entities[0];
                        Wallet &wallet = registry.wallets.get(player);
                        wallet.crackers += drops.cracker;

                        spdlog::info("{} collected {} crackers! Total: {}", player.name_id(), drops.cracker,
                                     wallet.crackers);
                    }

                    Motion &m = registry.motions.get(e);
                    createEffectSmoke(m.position);

                    registry.remove_all_components_of(e);
                    // registry.progresses.components[0].remaining_enemies -= 1;
                }
            }
        }
    }
}

void DeathSystem::changeEnemyType(Entity enemy, ENEMY_TYPE type) {
    switch (type) {
    case ENEMY_TYPE::GUARD:
        registry.beziers.remove(enemy);
        registry.leaders.remove(enemy);
        break;
    case ENEMY_TYPE::HOMING:
        registry.boids.remove(enemy);
        auto config = ENEMY_CONFIGS[(int)type];
        FireControl &fc = registry.fireControls.get(enemy);
        fc.cooldown_ms = ATTACK_CONFIGS[(int)config.attack_pattern].cooldown_ms;
        fc.attack_pattern = config.attack_pattern;
        break;
    }

    AI &ai = registry.ais.get(enemy);
    ai.tree_file = "enemy_tree.txt";
    ai.tree = load_tree(ai.tree_file);
    ai.current_state = AIAction::PURSUE;

}

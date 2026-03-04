#include "ote_system.hpp"

#include "systems/health/heal_damage_system.hpp"
#include <spdlog/spdlog.h>

void OverTimeEffectSystem::step(float elapsed_ms) {
    // Handle OverTimeEffect effects
    for (Entity e : registry.overTimeEffects.entities) {
        OverTimeEffect &ot = registry.overTimeEffects.get(e);
        ot.elapsed_time_ms += elapsed_ms;

        if (ot.elapsed_time_ms >= ot.interval_ms) {
            ot.elapsed_time_ms = 0;
            ot.duration_time_ms -= ot.interval_ms;

            if (registry.targets.has(e)) {
                Entity &target_entity = registry.targets.get(e).target;

                if (registry.heals.has(e))
                    handle_heal(e, target_entity);
                else if (registry.damages.has(e))
                    handle_damage(e, target_entity);
            }
        }

        // Effect expired
        if (ot.duration_time_ms <= 0) {
            registry.remove_all_components_of(e);
            spdlog::debug("{} has expired", e.name_id());
        }
    }
}

void OverTimeEffectSystem::handle_heal(Entity e, Entity target_entity) {
    // Remove effect if target is dead
    if (registry.deads.has(target_entity)) {
        registry.remove_all_components_of(e);
        return;
    }

    if (!registry.heals.has(target_entity)) {
        int amount = registry.heals.get(e).amount;
        HealDamageSystem::add_heal(target_entity, amount);
    }
}

void OverTimeEffectSystem::handle_damage(Entity e, Entity target_entity) {
    // Remove effect if target is dead
    if (registry.deads.has(target_entity)) {
        registry.remove_all_components_of(e);
        return;
    }

    if (!registry.damages.has(target_entity)) {
        int amount = registry.damages.get(e).amount;
        HealDamageSystem::add_damage(target_entity, amount);
    }
}

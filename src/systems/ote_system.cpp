#include "ote_system.hpp"

#include "heal_damage_system.hpp"

void OverTimeEffectSystem::step(float elapsed_ms) {
    // Handle OverTimeEffect effects
    for (Entity e : registry.overTimeEffect.entities) {
        OverTimeEffect &ot = registry.overTimeEffect.get(e);
        ot.elapsed_time_ms += elapsed_ms;

        if (ot.elapsed_time_ms >= ot.interval_ms) {
            ot.elapsed_time_ms = 0;
            ot.duration_time_ms -= ot.interval_ms;

            if (registry.target.has(e)) {
                Entity &target_entity = registry.target.get(e).target;

                if (registry.heal.has(e))
                    handle_heal(e, target_entity);
                else if (registry.damage.has(e))
                    handle_damage(e, target_entity);
            }
        }

        // Effect expired
        if (ot.duration_time_ms <= 0) {
            registry.remove_all_components_of(e);
            // std::cout << e.name_id() << " has expired" << std::endl;
        }
    }
}

void OverTimeEffectSystem::handle_heal(Entity e, Entity target_entity) {
    // Remove effect if target is dead
    if (registry.dead.has(target_entity)) {
        registry.remove_all_components_of(e);
        return;
    }

    if (!registry.heal.has(target_entity)) {
        int amount = registry.heal.get(e).amount;
        HealDamageSystem::add_heal(target_entity, amount);
    }
}

void OverTimeEffectSystem::handle_damage(Entity e, Entity target_entity) {
    // Remove effect if target is dead
    if (registry.dead.has(target_entity)) {
        registry.remove_all_components_of(e);
        return;
    }

    if (!registry.damage.has(target_entity)) {
        int amount = registry.damage.get(e).amount;
        HealDamageSystem::add_damage(target_entity, amount);
    }
}
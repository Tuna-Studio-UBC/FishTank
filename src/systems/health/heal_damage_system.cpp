#include "heal_damage_system.hpp"

#include <spdlog/spdlog.h>

void HealDamageSystem::step(float elapsed_ms) {
    (void)elapsed_ms;

    handle_damaging();
    handle_healing();
}

void HealDamageSystem::handle_damaging() {
    for (Entity e : registry.damage.entities) {
        if (registry.health.has(e)) {
            Damage &d = registry.damage.get(e);
            Health &h = registry.health.get(e);
            h.hp = std::max(h.hp - d.amount, 0);
            registry.damage.remove(e);

            // Handle visual indicators
            if (!registry.tints.has(e)) {
                registry.tints.emplace(e, Tint(damage_tint, tint_duration_ms));
            } else {
                Tint &tint = registry.tints.get(e);
                tint.tint = damage_tint;
                tint.life_ms = tint_duration_ms;
            }

            spdlog::info("{} took {} damage and now has {}/{} HP", e.name_id(), d.amount, h.hp, h.max_hp);
        }
    }
}

void HealDamageSystem::handle_healing() {
    for (Entity e : registry.heal.entities) {
        if (registry.health.has(e)) {
            Heal &h = registry.heal.get(e);
            Health &he = registry.health.get(e);
            he.hp = std::min(he.hp + h.amount, he.max_hp);
            registry.heal.remove(e);

            // TODO: Handle visual indicators
            if (!registry.tints.has(e)) {
                registry.tints.emplace(e, Tint(heal_tint, tint_duration_ms));
            } else {
                Tint &tint = registry.tints.get(e);
                tint.tint = heal_tint;
                tint.life_ms = tint_duration_ms;
            }

            spdlog::info("{} took {} healing and now has {}/{} HP", e.name_id(), h.amount, he.hp, he.max_hp);
        }
    }
}

// TODO: refactor to generic duplicate check if needed
void HealDamageSystem::add_damage(Entity entity, int amount) {
    if (!registry.damage.has(entity)) {
        registry.damage.emplace(entity, amount);
    } else {
        spdlog::debug("DUPLICATE DAMAGE WAS CALLED");
        Damage &d = registry.damage.get(entity);
        d.amount += amount;
    }
}

void HealDamageSystem::add_heal(Entity entity, int amount) {
    if (!registry.heal.has(entity)) {
        registry.heal.emplace(entity, amount);
    } else {
        spdlog::debug("DUPLICATE HEAL WAS CALLED");
        Heal &h = registry.heal.get(entity);
        h.amount += amount;
    }
}

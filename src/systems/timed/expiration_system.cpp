#include "expiration_system.hpp"

void ExpirationSystem::step(float elapsed_ms) {
    for (Entity e : registry.expirations.entities) {
        Expiration &exp = registry.expirations.get(e);
        exp.lifespan_ms -= elapsed_ms;

        if (exp.lifespan_ms <= 0) {
            spdlog::debug("{} has expired", e.name_id());
            registry.remove_all_components_of(e);
        }
    }

    for (Entity e : registry.tints.entities) {
        Tint &tint = registry.tints.get(e);

        if (tint.life_ms == -99.f)
            continue;

        tint.life_ms -= elapsed_ms;
        if (tint.life_ms <= 0)
            registry.tints.remove(e);
    }
}

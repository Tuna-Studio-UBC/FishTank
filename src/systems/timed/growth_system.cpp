#include "growth_system.hpp"

#include <glm/glm.hpp>

void GrowthSystem::step(float elapsed_ms) {
    for (Entity e : registry.growths.entities) {
        // Skip if no motion
        if (!registry.motions.has(e)) {
            registry.growths.remove(e);
            continue;
        }

        Motion &motion = registry.motions.get(e);
        Growth &growth = registry.growths.get(e);

        if (motion.scale.x < growth.target_scale.x && motion.scale.y < growth.target_scale.y) {
            motion.scale += growth.factor * (elapsed_ms / 1000.f) * growth.rate_ps;
        } else {
            registry.growths.remove(e);
        }
    }
}

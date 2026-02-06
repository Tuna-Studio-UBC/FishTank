#include <iostream>
#include "bullet_system.hpp"
#include "world_init.hpp"

void BulletSystem::step(float elapsed_ms)
{
	for (size_t i = registry.bullet.components.size(); i-- > 0; ) {
		Entity entity = registry.bullet.entities[i];
		Bullet& bullet = registry.bullet.components[i];

		bullet.lifespan_ms -= elapsed_ms;

		if (bullet.lifespan_ms <= 0) {
			// Print to console for now
			std::cout << "Bullet " << entity << " has been removed." << std::endl;
			registry.remove_all_components_of(entity);
		}
	}
}

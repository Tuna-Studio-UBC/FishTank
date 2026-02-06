#include "collision_system.hpp"

// Compute collisions between entities
void CollisionSystem::step(float elapsed_ms)
{

  ComponentContainer<Collision> &collision_registry = registry.collisions;
  for (uint i = 0; i < collision_registry.components.size(); i++)
  {
    Entity entity_a = collision_registry.entities[i];
    Entity entity_b = collision_registry.components[i].other;

    handle_bullets(entity_a, entity_b);
	handle_walls(entity_a, entity_b);
  }

  // Clear all collisions for the current step
  registry.collisions.clear();
}

void CollisionSystem::handle_bullets(Entity entity_a, Entity entity_b)
{
  // Bullet hit player
  if (registry.bullet.has(entity_a) && registry.players.has(entity_b))
  {
    Bullet &bullet = registry.bullet.get(entity_a);
    if (bullet.owner != entity_b)
    {
      registry.damage.emplace(entity_b, bullet.damage);
      registry.remove_all_components_of(entity_a); // remove bullet
    }
  }
  // Bullet hit enemy
  else if (registry.bullet.has(entity_a) && registry.enemies.has(entity_b))
  {
    Bullet &bullet = registry.bullet.get(entity_a);
    if (bullet.owner != entity_b && registry.health.has(entity_b))
    {
      registry.damage.emplace(entity_b, bullet.damage);
      registry.remove_all_components_of(entity_a); // remove bullet
    }
  }
  // Bullet hit bullet
  else if (registry.bullet.has(entity_a) && registry.bullet.has(entity_b))
  {
    registry.remove_all_components_of(entity_a); // remove bullet
    registry.remove_all_components_of(entity_b); // remove bullet
  }
}

void CollisionSystem::handle_walls(Entity entity_a, Entity entity_b) {
    if (registry.bullet.has(entity_a) && registry.wall.has(entity_b)) {
        if (registry.bouncy.has(entity_a)) {
            Motion &bullet_motion = registry.motions.get(entity_a);
            Motion& wall_motion = registry.motions.get(entity_b);
            vec2 wall_scale = wall_motion.scale;
            bool is_vertical_wall = wall_scale.x < wall_scale.y;
            if (is_vertical_wall) {
                bullet_motion.angle = M_PI - bullet_motion.angle;
            }
            else {
                bullet_motion.angle = -bullet_motion.angle;
            }

            while (bullet_motion.angle < 0) bullet_motion.angle += 2 * M_PI;
            while (bullet_motion.angle >= 2 * M_PI) bullet_motion.angle -= 2 * M_PI;
            std::cout << "Bullet bounced" << std::endl;
            
        }
    }
}

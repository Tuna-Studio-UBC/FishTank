#include "collision_system.hpp"
vec2 get_bounding_box(const Motion& motion);
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
            Motion& bullet_motion = registry.motions.get(entity_a);
            Motion& wall_motion = registry.motions.get(entity_b);

            // Calculate relative position to determine bounce direction
            vec2 bullet_to_wall = bullet_motion.position - wall_motion.position;
            vec2 wall_half_size = wall_motion.scale / 2.f;
            vec2 bullet_half_size = get_bounding_box(bullet_motion) / 2.f;

            // Determine which side of the wall was hit
            float overlap_x = wall_half_size.x - abs(bullet_to_wall.x);
            float overlap_y = wall_half_size.y - abs(bullet_to_wall.y);

			const float MIN_OVERLAP = 0.1f; // Minimum overlap to consider a collision (to avoid jitter)
            if (overlap_x < MIN_OVERLAP || overlap_y < MIN_OVERLAP) {
                // Not enough overlap, likely a corner case, skip bounce
                return;
			}
			vec2 velocity_dir = vec2(cos(bullet_motion.angle), sin(bullet_motion.angle));
            float overlap_diff = abs(overlap_x - overlap_y);
            if (overlap_diff < 2.f) {
                bullet_motion.angle = bullet_motion.angle + M_PI;
            }
            else if (overlap_x < overlap_y) {
                // Hit left or right side (vertical bounce)
                bullet_motion.angle = M_PI - bullet_motion.angle;
                // Push bullet out horizontally
                if (bullet_to_wall.x > 0)
                    bullet_motion.position.x = wall_motion.position.x + wall_half_size.x + bullet_half_size.x + 2.f;
                else
                    bullet_motion.position.x = wall_motion.position.x - wall_half_size.x - bullet_half_size.x - 2.f;
            }
            else {
                // Hit top or bottom (horizontal bounce)
                bullet_motion.angle = -bullet_motion.angle;
                // Push bullet out vertically
                if (bullet_to_wall.y > 0)
                    bullet_motion.position.y = wall_motion.position.y + wall_half_size.y + bullet_half_size.y + 2.f;
                else
                    bullet_motion.position.y = wall_motion.position.y - wall_half_size.y - bullet_half_size.y - 2.f;
            }

            // Normalize angle
            while (bullet_motion.angle < 0) bullet_motion.angle += 2 * M_PI;
            while (bullet_motion.angle >= 2 * M_PI) bullet_motion.angle -= 2 * M_PI;

            std::cout << "Bullet bounced off wall!" << std::endl;
        }
        else {
            // Non-bouncy bullets get destroyed
            registry.remove_all_components_of(entity_a);
        }
    }
}

void CollisionSystem::handle_entities_vs_walls(Entity entity_a, Entity entity_b) {
    // player vs wall
    if (registry.players.has(entity_a) && registry.wall.has(entity_b))
    {
        Motion &player_motion = registry.motions.get(entity_a);
        Motion& wall_motion = registry.motions.get(entity_b);

        // seperation vector to push player out of wall
        vec2 seperation = player_motion.position - wall_motion.position;
        vec2 wall_half_size = wall_motion.scale / 2.f;
        vec2 player_half_size = player_motion.scale / 2.f;

        // overlap
        float overlap_x = (wall_half_size.x + player_half_size.x) - abs(seperation.x);
        float overlap_y = (wall_half_size.y + player_half_size.y) - abs(seperation.y);

        // push player out along the axis
        if (overlap_x < overlap_y)
        {
            // horizontal push
            if (seperation.x > 0) player_motion.position.x += overlap_x;
            else player_motion.position.x -= overlap_x;
        }
        else {
            //vertical push
            if (seperation.y > 0) player_motion.position.y += overlap_y;
            else player_motion.position.y -= overlap_y;
        }
    }
}

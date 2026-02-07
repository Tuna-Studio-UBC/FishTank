// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
/*bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}*/

// AABB Collisions
bool collision_AABB(const Motion& motion1, const Motion& motion2)
{
	vec2 box1 = get_bounding_box(motion1);
	vec2 box2 = get_bounding_box(motion2);

	float left1 = motion1.position.x - box1.x / 2.f;
	float right1 = motion1.position.x + box1.x / 2.f;
	float top1 = motion1.position.y - box1.y / 2.f;
	float bottom1 = motion1.position.y + box1.y / 2.f;

	float left2 = motion2.position.x - box2.x / 2.f;
	float right2 = motion2.position.x + box2.x / 2.f;
	float top2 = motion2.position.y - box2.y / 2.f;
	float bottom2 = motion2.position.y + box2.y / 2.f;

	bool collisionX = right1 >= left2 && left1 <= right2;
	bool collisionY = bottom1 >= top2 && top1 <= bottom2;
	return collisionX && collisionY;
}
bool collides_with_tile(vec2 position, vec2 half_size, const TileMap& tilemap, const TileSet& tileset) {
	int min_tile_x = (int)floor((position.x - half_size.x) / TILE_SIZE_PX);
	int max_tile_x = (int)floor((position.x + half_size.x) / TILE_SIZE_PX);
	int min_tile_y = (int)floor((position.y - half_size.y) / TILE_SIZE_PX);
	int max_tile_y = (int)floor((position.y + half_size.y) / TILE_SIZE_PX);

	for (int ty = min_tile_y; ty <= max_tile_y; ty++) {
		for (int tx = min_tile_x; tx <= max_tile_x; tx++) {
			if (tx < 0 || tx >= (int)tilemap.width || ty < 0 || ty >= (int)tilemap.height)
				continue;
			unsigned short tile_id = tilemap.tile_ids[tx + ty * tilemap.width];
			//DEBUG: print tile info
			std::cout << "Tile (" << tx << "," << ty << ") - ID: " << tile_id
				<< " - Has Collision: " << (tile_id < 2048 && tileset.collisions[tile_id] ? "YES" : "NO")
				<< std::endl;
			if (tile_id < 2048 && tileset.collisions[tile_id])
				return true;

		}
	}
	return false;
}
void PhysicsSystem::step(float elapsed_ms)
{
	TileMap& tilemap = registry.tilemaps.components[0];
	TileSet& tileset = registry.tilesets.components[0];
	// Move each entity that has motion
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.components.size(); i++) // changed so i can use index to get entity, sorry
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		vec2 old_position = motion.position;

		auto velocity = motion.velocity * elapsed_ms / 1000.f;
		auto angle = motion.angle;
		float vx_prime = cos(angle) * velocity.x - sin(angle) * velocity.y;
		float vy_prime = sin(angle) * velocity.x + cos(angle) * velocity.y;
		//motion.position += vec2(vx_prime, vy_prime);

		vec2 movement = vec2(vx_prime, vy_prime);
		motion.position += movement;

		if (registry.players.has(entity) || registry.enemies.has(entity))
		{
			vec2 half_size = get_bounding_box(motion) / 2.f;
			if (collides_with_tile(motion.position, half_size, tilemap, tileset)) {
				motion.position = old_position; // try only moving horiontally
				motion.position.x += movement.x;

				if (collides_with_tile(motion.position, half_size, tilemap, tileset))
				{
					// x movement blocked, revert x
					motion.position.x = old_position.x;
				}
				// try moving only vertically
				motion.position.y = old_position.y + movement.y;
				if (collides_with_tile(motion.position, half_size, tilemap, tileset)) {
					// y movement blocked, revert y
					motion.position.y = old_position.y;
				}
			}
		}
	}
	// Clamp player to arena bounds
	for (Entity player_entity : registry.players.entities)
	{
		assert(motion_registry.has(player_entity) && "Player has no motion entity!");
		Motion& player_motion = motion_registry.get(player_entity);
		TileMap& tilemap = registry.tilemaps.components[0];
		vec2 half = get_bounding_box(player_motion) / 2.f;
		vec2 margin = vec2(TILE_SIZE_PX + half.x, TILE_SIZE_PX + half.y);
		float min_x = margin.x;
		float max_x = tilemap.width * TILE_SIZE_PX - margin.x;
		float min_y = margin.y;
		float max_y = tilemap.height * TILE_SIZE_PX - margin.y;

		player_motion.position.x = glm::clamp(player_motion.position.x, min_x, max_x);
		player_motion.position.y = glm::clamp(player_motion.position.y, min_y, max_y);
	}
	// check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collision_AABB(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				registry.collisions.insert(entity_i, {entity_j}, false);
				registry.collisions.insert(entity_j, {entity_i}, false); 
			}
		}
	}
}
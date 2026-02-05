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

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion
	auto& motion_registry = registry.motions;
	for(Motion& motion : motion_registry.components)
	{
		auto velocity = motion.velocity * elapsed_ms / 1000.f;
		auto angle = motion.angle;
		float vx_prime = cos(angle) * velocity.x - sin(angle) * velocity.y;
		float vy_prime = sin(angle) * velocity.x + cos(angle) * velocity.y;
		motion.position += vec2(vx_prime, vy_prime);
	}
	// Clamp player to arena bounds
	for (Entity player_entity : registry.players.entities)
	{
		if (motion_registry.has(player_entity))
		{
			Motion& player_motion = motion_registry.get(player_entity);
			vec2 half = get_bounding_box(player_motion) / 2.f;
			float margin = TILE_SIZE_PX;
			float min_x = margin + half.x;
			float max_x = WINDOW_WIDTH_PX - margin - half.x;
			float min_y = margin + half.y;
			float max_y = WINDOW_HEIGHT_PX - margin - half.y;

			player_motion.position.x = glm::clamp(player_motion.position.x, min_x, max_x);
			player_motion.position.y = glm::clamp(player_motion.position.y, min_y, max_y);
		}
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
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i); // uncommented, to have both entities know about the collision
			}
		}
	}
}
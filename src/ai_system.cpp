#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

bool player_in_radius(vec2 player_pos, Motion enemy_motion, float radius) {
	return player_pos.x <= enemy_motion.position.x + enemy_motion.scale.x + radius &&
		player_pos.x >= enemy_motion.position.x - enemy_motion.scale.x - radius &&
		player_pos.y <= enemy_motion.position.y + enemy_motion.scale.y + radius &&
		player_pos.y >= enemy_motion.position.y - enemy_motion.scale.y - radius;
}

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	ComponentContainer<Detected>& detected_container = registry.detecteds;
	Entity player = registry.players.entities[0];
	vec2 player_pos = registry.motions.get(player).position;

	for (uint i = 0; i < detected_container.components.size(); i++)
	{
		Entity enemy_i = detected_container.entities[i];
		Motion  &motion_i = registry.motions.get(enemy_i);

		float target_angle = atan2(player_pos.y - motion_i.position.y, player_pos.x - motion_i.position.x);
		float diff = target_angle - motion_i.angle;

		float turn_speed = elapsed_ms / 1000.f;
		motion_i.angle = diff * turn_speed;

		motion_i.position = vec2(motion_i.position.x + motion_i.velocity.x, motion_i.position.y);
	}
}

void AISystem::handle_detection() {
	ComponentContainer<Deadly>& deadly_container = registry.deadlys;
	Entity player = registry.players.entities[0];
	vec2 player_pos = registry.motions.get(player).position;

	for (uint i = 0; i < deadly_container.components.size(); i++)
	{
		float radius_i = deadly_container.components[i].radius;
		Entity enemy_i = deadly_container.entities[i];
		Motion &motion_i = registry.motions.get(enemy_i);

		if (!registry.detecteds.has(enemy_i) && player_in_radius(player_pos, motion_i, radius_i)) {
			registry.detecteds.emplace(enemy_i);
			motion_i.velocity = vec2(1.f, 0.f);

			// log to console for now
			std::cout << "player detected by enemy " << enemy_i.id() << std::endl;
		}
		if (registry.detecteds.has(enemy_i) && !player_in_radius(player_pos, motion_i, radius_i)) {
			registry.detecteds.remove(enemy_i);
			motion_i.velocity = vec2(0.f, 0.f);

			// log to console for now
			std::cout << "player no longer detected by enemy " << enemy_i.id() << std::endl;
		}
	}
}
#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

static bool player_in_radius(vec2 player_pos, Motion enemy_motion, float radius) {
	return player_pos.x <= enemy_motion.position.x + enemy_motion.scale.x + radius &&
		player_pos.x >= enemy_motion.position.x - enemy_motion.scale.x - radius &&
		player_pos.y <= enemy_motion.position.y + enemy_motion.scale.y + radius &&
		player_pos.y >= enemy_motion.position.y - enemy_motion.scale.y - radius;
}

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	ComponentContainer<Pursuit>& pursuit_container = registry.pursuit;
	Entity player = registry.players.entities[0];
	vec2 player_pos = registry.motions.get(player).position;

	float elapsed_s = elapsed_ms / 1000.f;

	for (uint i = 0; i < pursuit_container.components.size(); i++)
	{
		Entity enemy_i = pursuit_container.entities[i];
		Motion  &motion_i = registry.motions.get(enemy_i);
		SpeedControl &speed_i = registry.speedControl.get(enemy_i);

		float target_angle = atan2(player_pos.y - motion_i.position.y, player_pos.x - motion_i.position.x) * 180.f / 3.14;
		float diff = target_angle - motion_i.angle;
		while (diff > 180.f) {
			diff -= 360.f;
		}
		while (diff < -180.f) {
			diff += 360.f;
		}

		float maxStep = speed_i.turn_speed * elapsed_s;

		float step = std::clamp(diff, -maxStep, maxStep);

		motion_i.angle += step;

		if (motion_i.angle >= 360.f) {
			motion_i.angle -= 360.f;
		}
		if (motion_i.angle < 0.f) {
			motion_i.angle += 360.f;
		}
	}
}

void AISystem::handle_detection() {
	ComponentContainer<Enemy>& enemy_container = registry.enemies;
	Entity player = registry.players.entities[0];
	vec2 player_pos = registry.motions.get(player).position;

	for (uint i = 0; i < enemy_container.components.size(); i++)
	{
		float radius_i = enemy_container.components[i].detection_radius;
		Entity enemy_i = enemy_container.entities[i];
		Motion &motion_i = registry.motions.get(enemy_i);

		if (!registry.pursuit.has(enemy_i) && player_in_radius(player_pos, motion_i, radius_i)) {
			registry.pursuit.emplace(enemy_i);
			motion_i.velocity = vec2(1.f, 0.f);

			// log to console for now
			std::cout << "player detected by enemy " << enemy_i.id() << std::endl;
		}
		if (registry.pursuit.has(enemy_i) && !player_in_radius(player_pos, motion_i, radius_i)) {
			registry.pursuit.remove(enemy_i);
			motion_i.velocity = vec2(0.f, 0.f);

			// log to console for now
			std::cout << "player no longer detected by enemy " << enemy_i.id() << std::endl;
		}
	}
}
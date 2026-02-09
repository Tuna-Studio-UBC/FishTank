#include <iostream>
#include "systems/ai_system.hpp"
#include "world_init.hpp"
#include <cmath>
#include <random>

static vec2 direction_vector(vec2 v, vec2 fallback)
{
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len < 1e-5f) return fallback;
    return v / len;
}

float adjust_angle(Entity enemy, vec2 target_pos, float elapsed_ms, float stopping_dist) {
	Motion& motion = registry.motions.get(enemy);
	vec2 position = motion.position;
	float angle = motion.angle;
	SpeedControl& speed = registry.speedControl.get(enemy);

	// TODO: not hardcode stopping distance
	if (entity_in_radius(target_pos - motion.position, motion.scale, stopping_dist)) {
		motion.velocity = vec2(0.f, 0.f);
	}
	else {
		motion.velocity = vec2(speed.move_speed, 0.f);
	}

	// Adjust angle to track target
	float target_angle_radians = atan2(target_pos.y - position.y, target_pos.x - position.x);
	float max_turn = speed.turn_speed * (elapsed_ms / 1000.f);

	float current_angle = motion.angle;
	float diff = target_angle_radians - current_angle;
	while (diff > M_PI) {
		diff -= 2 * M_PI;
	}
	while (diff < -M_PI) {
		diff += 2 * M_PI;
	}

	float step = std::clamp(diff, -max_turn, max_turn);

	current_angle += step;

	if (current_angle >= 2 * M_PI) {
		current_angle -= 2 * M_PI;
	}
	if (current_angle < 0.f) {
		current_angle += 2 * M_PI;
	}
	motion.angle = current_angle;
	return motion.angle;
}

// COMMENTED OUT FOR NOW, unsure if we will keep patrolling yet
// IF UNCOMMENT, requires refactoring
/*void patrol(float elapsed_ms) {
	float dt = elapsed_ms / 1000.f;
	static std::mt19937 rng{ std::random_device{}() };
	static std::uniform_real_distribution<float> angle_dist(0.f, 2.f * (float)M_PI);
	static std::uniform_real_distribution<float> t01(0.f, 1.f);

	for (Entity e : registry.enemies.entities)
	{
		if (!registry.motions.has(e) || !registry.patrols.has(e))
			continue;

		Motion& m = registry.motions.get(e);
		Patrol& p = registry.patrols.get(e);
		p.time_to_change_ms -= elapsed_ms;

		if (p.time_to_change_ms <= 0.f)
		{
			float a = angle_dist(rng);
			p.dir = { cosf(a), sinf(a) };
			p.dir = direction_vector(p.dir, { 1.f, 0.f });

			float span = p.change_dir_max_ms - p.change_dir_min_ms;
			p.time_to_change_ms = p.change_dir_min_ms + t01(rng) * span;
		}

		m.velocity = p.dir * p.speed_px_s;
		m.position += m.velocity * dt;

		if (fabs(m.velocity.x) + fabs(m.velocity.y) > 1e-3f)
			m.angle = atan2f(m.velocity.y, m.velocity.x) * (180.f / (float)M_PI);

		float min_x = 0.f + p.padding_px;
		float max_x = WINDOW_WIDTH_PX - p.padding_px;
		float min_y = 0.f + p.padding_px;
		float max_y = WINDOW_HEIGHT_PX - p.padding_px;

		bool bounced = false;

		if (m.position.x < min_x) { m.position.x = min_x; p.dir.x *= -1.f; bounced = true; }
		if (m.position.x > max_x) { m.position.x = max_x; p.dir.x *= -1.f; bounced = true; }
		if (m.position.y < min_y) { m.position.y = min_y; p.dir.y *= -1.f; bounced = true; }
		if (m.position.y > max_y) { m.position.y = max_y; p.dir.y *= -1.f; bounced = true; }

		if (bounced)
		{
			p.dir = direction_vector(p.dir, { 1.f, 0.f });
		}
	}
}*/

void AISystem::step(float elapsed_ms)
{
	// COMMENTED OUT FOR NOW, unsure if we will keep patrolling yet
	// patrol(elapsed_ms);
	ComponentContainer<Distracted>& distracted_container = registry.distracted;
	Entity player = registry.players.entities[0];
	vec2 target_pos;

	for (uint i = 0; i < distracted_container.components.size(); i++)
	{
		Entity enemy_i = distracted_container.entities[i];
		Distracted& distracted = distracted_container.get(enemy_i);
		distracted.remaining_ms -= elapsed_ms;
		float distracted_remaining = distracted.remaining_ms;

		if (distracted.remaining_ms > 0) {
			target_pos = registry.motions.get(distracted.bell).position;
			adjust_angle(enemy_i, target_pos, elapsed_ms, 50.f);
		}
		else {
			distracted_container.remove(enemy_i);
		}
	}

	ComponentContainer<Pursuit>& pursuit_container = registry.pursuit;

	for (uint i = 0; i < pursuit_container.components.size(); i++)
	{
		Entity enemy_i = pursuit_container.entities[i];
		target_pos = registry.motions.get(player).position;
		
		float angle = adjust_angle(enemy_i, target_pos, elapsed_ms, 100.f);
		
		// Firing logic
		FireControl& fire = registry.fireControl.get(enemy_i);
		fire.cooldown_timer += elapsed_ms;
		vec2 position = registry.motions.get(enemy_i).position;

		if (fire.cooldown_timer >= fire.cooldown_ms) {
			if (fire.attack_pattern == ATTACK_PATTERN::URCHIN) {
				float incAngle = 0.f;
				for (int i = 0; i < 8; i++) {
					auto& req = registry.bullet_requests.emplace_with_duplicates(enemy_i);
					req.attack_pattern = fire.attack_pattern;
					req.position = position;
					req.angle = incAngle;
					req.owner = enemy_i; //this is new, to prevent self hit
					incAngle += M_PI / 4.f;
				}
			}
			else {
				registry.bullet_requests.insert(
					enemy_i,
					{
						fire.attack_pattern,
						position,
						angle,
						enemy_i // to prevent self hit
					});
			}
			fire.cooldown_timer = 0;
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

		bool in_radius = entity_in_radius(player_pos - motion_i.position, motion_i.scale, radius_i);
    
		if (!registry.pursuit.has(enemy_i) && in_radius && !registry.distracted.has(enemy_i)) {
			registry.pursuit.emplace(enemy_i);
			float speed = registry.speedControl.get(enemy_i).move_speed;
			motion_i.velocity = vec2(speed, 0.f);

			// log to console for now
			std::cout << enemy_i.name_id() << " detected " << player.name_id() << std::endl;
		}
		if (registry.pursuit.has(enemy_i) && !in_radius) {
			registry.pursuit.remove(enemy_i);
			motion_i.velocity = vec2(0.f, 0.f);

			// log to console for now
			std::cout << enemy_i.name_id() << " no longer detecting " << player.name_id() << std::endl;
		}
	}
}
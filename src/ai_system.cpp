#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include <cmath>
#include <random>

static vec2 direction_vector(vec2 v, vec2 fallback)
{
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len < 1e-5f) return fallback;
    return v / len;
}

void adjust_angle(float target_angle, Motion &motion, float max_turn) {
	float current_angle = motion.angle;
	float diff = target_angle - current_angle;
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

	ComponentContainer<Pursuit>& pursuit_container = registry.pursuit;
	Entity player = registry.players.entities[0];
	vec2 target_pos = registry.motions.get(player).position;

	for (uint i = 0; i < pursuit_container.components.size(); i++)
	{
		Entity enemy_i = pursuit_container.entities[i];

		if (registry.distracted.has(enemy_i)) {
			Distracted& distracted = registry.distracted.get(enemy_i);
			distracted.remaining_ms -= elapsed_ms;

			if (distracted.remaining_ms > 0) {
				target_pos = registry.motions.get(distracted.bell).position;
			}
			else {
				registry.distracted.remove(enemy_i);
			}
		}

		Motion  &motion_i = registry.motions.get(enemy_i);
		vec2 position = motion_i.position;
		float angle = motion_i.angle;
		SpeedControl &speed = registry.speedControl.get(enemy_i);
		
		// TODO: not hardcode stopping distance
		if (entity_in_radius(target_pos - motion_i.position, motion_i.scale, 100.f)) {
			motion_i.velocity = vec2(0.f,0.f);
		}
		else {
			motion_i.velocity = vec2(speed.move_speed, 0.f);
		}

		// Adjust angle to track player
		float target_angle = atan2(target_pos.y - position.y, target_pos.x - position.x);
		float max_turn = speed.turn_speed * (elapsed_ms / 1000.f);
		adjust_angle(target_angle, motion_i, max_turn);
		
		// Firing logic
		FireControl& fire = registry.fireControl.get(enemy_i);
		fire.cooldown_timer += elapsed_ms;

		if (fire.cooldown_timer >= fire.cooldown_ms) {
			if (fire.attack_pattern == ATTACK_PATTERN::URCHIN) {
				float angle = 0.f;
				for (int i = 0; i < 6; i++) {
					auto& req = registry.bullet_requests.emplace_with_duplicates(enemy_i);
					req.attack_pattern = fire.attack_pattern;
					req.position = position;
					req.angle = angle;
					req.owner = enemy_i; //this is new, to prevent self hit
					angle += 45.f;
				}
			}
			else {
				registry.bullet_requests.insert(
					enemy_i,
					{
						fire.attack_pattern,
						position,
						motion_i.angle,
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
    
		if (!registry.pursuit.has(enemy_i) && in_radius) {
			registry.pursuit.emplace(enemy_i);
			float speed = registry.speedControl.get(enemy_i).move_speed;
			motion_i.velocity = vec2(speed, 0.f);

			// log to console for now
			std::cout << "player detected by enemy " << enemy_i.id() << std::endl;
		}
		if (registry.pursuit.has(enemy_i) && !in_radius) {
			registry.pursuit.remove(enemy_i);
			motion_i.velocity = vec2(0.f, 0.f);

			// log to console for now
			std::cout << "player no longer detected by enemy " << enemy_i.id() << std::endl;
		}
	}
}
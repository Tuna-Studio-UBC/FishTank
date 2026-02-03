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

// void AISystem::step(float elapsed_ms)
// {
// 	(void)elapsed_ms; // placeholder to silence unused warning until implemented

// }

static bool player_in_radius(vec2 player_pos, Motion enemy_motion, float radius) {
	return player_pos.x <= enemy_motion.position.x + enemy_motion.scale.x + radius &&
		player_pos.x >= enemy_motion.position.x - enemy_motion.scale.x - radius &&
		player_pos.y <= enemy_motion.position.y + enemy_motion.scale.y + radius &&
		player_pos.y >= enemy_motion.position.y - enemy_motion.scale.y - radius;
}

void AISystem::step(float elapsed_ms)
{
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
            p.dir = direction_vector(p.dir, {1.f, 0.f});

            float span = p.change_dir_max_ms - p.change_dir_min_ms;
            p.time_to_change_ms = p.change_dir_min_ms + t01(rng) * span;
        }

        m.velocity = p.dir * p.speed_px_s;
        m.position += m.velocity * dt;

        float min_x = 0.f + p.padding_px;
        float max_x = WINDOW_WIDTH_PX  - p.padding_px;
        float min_y = 0.f + p.padding_px;
        float max_y = WINDOW_HEIGHT_PX - p.padding_px;

        bool bounced = false;

        if (m.position.x < min_x) { m.position.x = min_x; p.dir.x *= -1.f; bounced = true; }
        if (m.position.x > max_x) { m.position.x = max_x; p.dir.x *= -1.f; bounced = true; }
        if (m.position.y < min_y) { m.position.y = min_y; p.dir.y *= -1.f; bounced = true; }
        if (m.position.y > max_y) { m.position.y = max_y; p.dir.y *= -1.f; bounced = true; }

        if (bounced)
        {
            p.dir = direction_vector(p.dir, {1.f, 0.f});
        }
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

        if (fabs(m.velocity.x) + fabs(m.velocity.y) > 1e-3f)
            m.angle = atan2f(m.velocity.y, m.velocity.x) * (180.f / (float)M_PI);
    }
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
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

        if (fabs(m.velocity.x) + fabs(m.velocity.y) > 1e-3f)
            m.angle = atan2f(m.velocity.y, m.velocity.x) * (180.f / (float)M_PI);
    }
}
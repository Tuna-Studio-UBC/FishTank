#include "swarm_system.hpp"

void generate_bezier(vec2 start_pos, vec2 forward_dir, BezierPath &curve) {
    float length = get_random_float(150.f, 400.f);

    vec2 projected = start_pos + forward_dir * length;
    const int max_attempts = 16;
    float angle_step = 3.14 / max_attempts;

    vec2 base_dir = forward_dir;
    bool found_dir = false;

    for (int i = 0; i < max_attempts; ++i) {
        float sign = (i % 2 == 0) ? 1.f : -1.f;
        float angle = sign * angle_step * (i / 2);

        vec2 rotated = vec2(base_dir.x * cos(angle) - base_dir.y * sin(angle), base_dir.x * sin(angle) + base_dir.y * cos(angle));

        projected = start_pos + normalize(rotated) * length;

        /* if (!is_out_of_bounds(world_to_tile(projected))) {
            forward_dir = normalize(rotated);
            break;
        }*/
    }

    vec2 right = vec2(-forward_dir.y, forward_dir.x);

    float lateral_offset = length * 0.3f;

    curve.p0 = start_pos;
    curve.p1 = start_pos + forward_dir * (length * 0.3f) + right * get_random_float(-lateral_offset, lateral_offset);
    curve.p2 = start_pos + forward_dir * (length * 0.6f) + right * get_random_float(-lateral_offset, lateral_offset);
    curve.p3 = start_pos + forward_dir * length;

    curve.speed = 0.4f + get_random_float(0.f, 0.3f);
}

vec2 evaluate_cubic_bezier(const BezierPath &curve, float t) {
    float u = 1.f - t;

    return u * u * u * curve.p0 + 3.f * u * u * t * curve.p1 + 3.f * u * t * t * curve.p2 + t * t * t * curve.p3;
}

void update_leader(Entity e, Motion &motion, float elapsed_ms) {
    BezierPath &curve = registry.beziers.get(e);
    if (curve.t >= 1.f) {
        curve.t = 0.f;
        curve.curves_left -= 1;
    } 
    if (curve.curves_left == 0) {
        registry.senses.get(e).is_patrolling = false;
        return;
    }
    if (curve.t == 0.f) {
        generate_bezier(motion.position, direction_to_unit_vector(motion.angle), curve);
        //spdlog::debug("generate new curve ({},{}) ({},{}) ({},{}) ({},{})", curve.p0.x, curve.p0.y, curve.p1.x, curve.p1.y, curve.p2.x, curve.p2.y, curve.p3.x, curve.p3.y);
    }

    curve.t += curve.speed * (elapsed_ms / 1000.f);
    curve.t = std::clamp(curve.t, 0.f, 1.f);

    vec2 target = evaluate_cubic_bezier(curve, curve.t);

    float angle = atan2(target.y - motion.position.y, target.x - motion.position.x);

    motion.angle = angle;
    motion.velocity = vec2(50.f, 0.f);
}

vec2 compute_alignment(Entity e, std::vector<Entity> &flock) {
    Boid &self = registry.boids.get(e);
    Motion &motion = registry.motions.get(e);

    vec2 avg = vec2(0.f);
    int count = 0;

    for (Entity other : flock) {
        Motion &other_motion = registry.motions.get(other);

        if (length(other_motion.position - motion.position) < self.perception_radius) {
            vec2 dir = vec2(cos(other_motion.angle), sin(other_motion.angle));
            avg += dir;
            count++;
        }
    }

    if (count == 0)
        return vec2(0.f);

    return normalize(avg / (float)count);
}

vec2 compute_cohesion(Entity e, std::vector<Entity> &flock) {
    Boid &self = registry.boids.get(e);
    Motion &motion = registry.motions.get(e);

    vec2 center = vec2(0.f);
    int count = 0;

    for (Entity other : flock) {
        Motion &other_motion = registry.motions.get(other);

        if (length(other_motion.position - motion.position) < self.perception_radius) {
            center += other_motion.position;
            count++;
        }
    }

    if (count == 0)
        return vec2(0.f);

    center /= (float)count;
    return normalize(center - motion.position);
}

vec2 compute_separation(Entity e, std::vector<Entity> &flock) {
    Boid &self = registry.boids.get(e);
    Motion &motion = registry.motions.get(e);

    vec2 force = vec2(0.f);

    for (Entity other : flock) {
        Motion &other_motion = registry.motions.get(other);

        vec2 diff = motion.position - other_motion.position;
        float dist = length(diff);

        if (dist > 0.f && dist < self.perception_radius * 0.5f) {
            force += normalize(diff) / dist;
        }
    }

    return normalize(force);
}

vec2 compute_boid_force(Entity e, std::vector<Entity> &flock) {
    Boid &boid = registry.boids.get(e);

    vec2 align = compute_alignment(e, flock) * boid.alignment_weight;
    vec2 cohesion = compute_cohesion(e, flock) * boid.cohesian_weight;
    vec2 separation = compute_separation(e, flock) * boid.separation_weight;

    vec2 total = align + cohesion + separation;

    if (length(total) == 0.f)
        return vec2(0.f);

    return normalize(total);
}

void update_follower(Entity follower, Entity leader, float elapsed_ms) {
    Motion &motion = registry.motions.get(follower);

    std::vector<Entity> flock = {leader};
    for (Entity bird : registry.boids.entities) {
        if (bird == follower) {
            continue;
        }
        if (registry.boids.get(bird).leader != leader) {
            continue;
        }
        flock.push_back(bird);
    }

    vec2 steering = compute_boid_force(follower, flock);

    vec2 dir_to_leader = normalize(registry.motions.get(leader).position - motion.position);

    vec2 final_dir = normalize(dir_to_leader + steering);

    motion.angle = atan2(final_dir.y, final_dir.x);
    motion.velocity = vec2(50.f, 0.f);
}

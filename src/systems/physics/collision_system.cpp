#include "collision_system.hpp"
#include "mesh_collision.hpp"
#include "systems/health/heal_damage_system.hpp"
#include "systems/world/world_init.hpp"
#include <spdlog/spdlog.h>

// Toggle for mesh collider, 0 = on.
static int MESH_COLLISION_MODE = 0; // set to any other value to turn it off and use AABB

vec2 get_bounding_box(const Motion &motion);

// Compute collisions between entities
void CollisionSystem::step(float elapsed_ms) {
    ComponentContainer<Collision> &collision_registry = registry.collisions;
    for (uint i = 0; i < collision_registry.components.size(); i++) {
        Entity entity_a = collision_registry.entities[i];
        Entity entity_b = collision_registry.components[i].other;

        // narrow-phase check: if either has a mesh collider, run mesh-based test
        bool passNarrowPhase = true;
        bool mesh_enabled = (MESH_COLLISION_MODE == 0);
        bool is_entity_a_mesh = mesh_enabled && registry.meshColliders.has(entity_a);
        bool is_entity_b_mesh = mesh_enabled && registry.meshColliders.has(entity_b);
        if (is_entity_a_mesh && is_entity_b_mesh) {
            // mesh vs mesh
            MeshCollider &mca = registry.meshColliders.get(entity_a);
            MeshCollider &mcb = registry.meshColliders.get(entity_b);
            Motion &motionA = registry.motions.get(entity_a);
            Motion &motionB = registry.motions.get(entity_b);
            auto polyA_world = transformPolygon(mca.vertices, motionA, mca.authoring_px);
            auto polyB_world = transformPolygon(mcb.vertices, motionB, mcb.authoring_px);
            vec2 tmpMtv;
            passNarrowPhase = computePolygonMTV(polyA_world, polyB_world, tmpMtv);
        } else if (is_entity_a_mesh || is_entity_b_mesh) {
            Entity meshed_entity = is_entity_a_mesh ? entity_a : entity_b;
            Entity non_meshed_entity = is_entity_a_mesh ? entity_b : entity_a;

            MeshCollider &mc = registry.meshColliders.get(meshed_entity);
            Motion &motionM = registry.motions.get(meshed_entity);

            // identify shapes
            if (registry.bullets.has(non_meshed_entity)) {
                // mesh vs bullet
                Motion &mBullet = registry.motions.get(non_meshed_entity);
                float radius = std::max(get_bounding_box(mBullet).x, get_bounding_box(mBullet).y) / 2.f;
                passNarrowPhase =
                    circleIntersectsPolygon(mBullet.position, radius, mc.vertices, motionM, mc.authoring_px);
            } else if (registry.walls.has(non_meshed_entity)) {
                // mesh vs wall -> use SAT on world-space polygons (MTV presence = intersection)
                Motion &mWall = registry.motions.get(non_meshed_entity);
                auto polyA = transformPolygon(mc.vertices, motionM, mc.authoring_px);
                vec2 half = mWall.scale / 2.f;
                std::vector<vec2> aabbPoly = {
                    mWall.position + vec2{-half.x, -half.y}, mWall.position + vec2{half.x, -half.y},
                    mWall.position + vec2{half.x, half.y}, mWall.position + vec2{-half.x, half.y}};
                vec2 tmpMtv;
                passNarrowPhase = computePolygonMTV(polyA, aabbPoly, tmpMtv);
            }
        }

        if (!passNarrowPhase)
            continue; // ignore false-positive broadphase

        handle_bullets(entity_a, entity_b);
        handle_walls(entity_a, entity_b);
        handle_actors_vs_walls(entity_a, entity_b);
        handle_hazards(entity_a, entity_b);
        handle_actor_actor(entity_a, entity_b);
        handle_bubbler_collision(entity_a, entity_b, elapsed_ms);
        handle_tutorial_collision(entity_a, entity_b);
    }

    // Clear all collisions for the current step
    registry.collisions.clear();
}

// Player or Enemy
bool CollisionSystem::is_actor(Entity entity) { return registry.players.has(entity) || registry.enemies.has(entity); }

void CollisionSystem::handle_bullets(Entity bullet_entity, Entity other_entity) {
    if (registry.bullets.has(bullet_entity)) {
        Bullet &bullet = registry.bullets.get(bullet_entity);
        // Bullet hit player
        if (registry.players.has(other_entity)) {
            if (bullet.owner != other_entity) {
                HealDamageSystem::add_damage(other_entity, bullet.damage);
                registry.remove_all_components_of(bullet_entity);
            }
        }
        // Bullet hit enemy
        else if (registry.enemies.has(other_entity)) {
            if (registry.players.has(bullet.owner)) { // Only player bullets damage enemies
                HealDamageSystem::add_damage(other_entity, bullet.damage);

                // Temp use bullet collision as trigger
                Motion &m = registry.motions.get(bullet_entity);
                createEffectBubbles(m.position);

                registry.remove_all_components_of(bullet_entity);
            }
        }
        // TODO: Enable if we don't like no-cancellations
        // // Bullet hit bullet
        // else if (registry.bullet.has(other_entity)) {
        //     Entity other_bullet_owner = registry.bullet.get(other_entity).owner;
        //     if (bullet.owner != other_bullet_owner &&
        //         !(registry.enemies.has(bullet.owner) &&
        //           registry.enemies.has(other_bullet_owner))) { // Ignore between enemies
        //         registry.remove_all_components_of(bullet_entity);
        //         registry.remove_all_components_of(other_entity);
        //     }
        // }
    }
}

// TODO: Handle applying hazards FX
void CollisionSystem::handle_hazards(Entity hazard_entity, Entity other_entity) {
    if (registry.hazards.has(hazard_entity)) {
        Hazard &hazard = registry.hazards.get(hazard_entity);
        auto &config = HAZARD_CONFIGS[(int)hazard.type];

        // Handle hazards against player
        if (registry.players.has(other_entity)) {
            if (config.duration_ms > 0.f && config.interval_ms > 0.f) {
                createOverTimeEffect<Damage>(other_entity, config.interval_ms, config.duration_ms, config.amount);
                registry.remove_all_components_of(hazard_entity);
            } else {
                HealDamageSystem::add_damage(other_entity, config.amount);
                registry.remove_all_components_of(hazard_entity);
            }
        }
        // Handle hazards against enemy
        else if (registry.enemies.has(other_entity)) {
            HealDamageSystem::add_damage(other_entity, config.amount);
            registry.remove_all_components_of(hazard_entity);
        }
    }
}

void CollisionSystem::handle_walls(Entity entity_a, Entity entity_b) {
    if (registry.bullets.has(entity_a) && registry.walls.has(entity_b)) {
        if (registry.bouncies.has(entity_a)) {
            Motion &bullet_motion = registry.motions.get(entity_a);
            Motion &wall_motion = registry.motions.get(entity_b);

            // Treat bullet as circle and wall as AABB: compute closest point on wall to bullet center
            vec2 bullet_pos = bullet_motion.position;
            vec2 wall_half_size = wall_motion.scale / 2.f;
            vec2 wall_min = wall_motion.position - wall_half_size;
            vec2 wall_max = wall_motion.position + wall_half_size;
            vec2 bullet_half_size = get_bounding_box(bullet_motion) / 2.f;
            float radius = std::max(bullet_half_size.x, bullet_half_size.y);

            // Closest point on the AABB to the bullet center
            vec2 closest;
            closest.x = std::max(wall_min.x, std::min(bullet_pos.x, wall_max.x));
            closest.y = std::max(wall_min.y, std::min(bullet_pos.y, wall_max.y));

            vec2 normal = bullet_pos - closest;
            float dist = length(normal);

            const float PUSH_OUT = 2.0f; // how far to push bullet out of wall (tweakable)
            const float EPS = 1e-4f;

            if (dist > EPS) {
                // normalized collision normal from wall toward bullet
                normal = normal / dist;

                // reflect velocity around the computed normal (realistic corner handling)
                vec2 velocity_dir = vec2(cos(bullet_motion.angle), sin(bullet_motion.angle));
                vec2 reflected = velocity_dir - 2.0f * dot(velocity_dir, normal) * normal;
                bullet_motion.angle = atan2(reflected.y, reflected.x);

                // Place bullet just outside the wall along the normal to avoid any penetration
                bullet_motion.position = closest + normal * (radius + PUSH_OUT);
            } else {
                // Bullet center is inside the rect (rare). Fall back to axis-based correction but still reflect using
                // axis normal.
                vec2 bullet_to_wall = bullet_motion.position - wall_motion.position;
                vec2 abs_sep = vec2(std::abs(bullet_to_wall.x), std::abs(bullet_to_wall.y));
                float overlap_x = wall_half_size.x - abs_sep.x;
                float overlap_y = wall_half_size.y - abs_sep.y;

                vec2 velocity_dir = vec2(cos(bullet_motion.angle), sin(bullet_motion.angle));

                if (overlap_x < overlap_y) {
                    // Push out horizontally and reflect across vertical normal
                    if (bullet_to_wall.x > 0)
                        bullet_motion.position.x = wall_motion.position.x + wall_half_size.x + radius + PUSH_OUT;
                    else
                        bullet_motion.position.x = wall_motion.position.x - wall_half_size.x - radius - PUSH_OUT;

                    vec2 axis_normal = vec2((bullet_to_wall.x > 0) ? 1.f : -1.f, 0.f);
                    vec2 reflected = velocity_dir - 2.0f * dot(velocity_dir, axis_normal) * axis_normal;
                    bullet_motion.angle = atan2(reflected.y, reflected.x);
                } else {
                    // Push out vertically and reflect across horizontal normal
                    if (bullet_to_wall.y > 0)
                        bullet_motion.position.y = wall_motion.position.y + wall_half_size.y + radius + PUSH_OUT;
                    else
                        bullet_motion.position.y = wall_motion.position.y - wall_half_size.y - radius - PUSH_OUT;

                    vec2 axis_normal = vec2(0.f, (bullet_to_wall.y > 0) ? 1.f : -1.f);
                    vec2 reflected = velocity_dir - 2.0f * dot(velocity_dir, axis_normal) * axis_normal;
                    bullet_motion.angle = atan2(reflected.y, reflected.x);
                }
            }

            // Normalize angle into [0, 2*pi)
            while (bullet_motion.angle < 0)
                bullet_motion.angle += 2 * M_PI;
            while (bullet_motion.angle >= 2 * M_PI)
                bullet_motion.angle -= 2 * M_PI;

        } else {
            // Non-bouncy bullets get destroyed
            registry.remove_all_components_of(entity_a);
        }
    }
}

void CollisionSystem::handle_actors_vs_walls(Entity entity_a, Entity entity_b) {
    if (is_actor(entity_a) && registry.walls.has(entity_b)) {
        Motion &actor_motion = registry.motions.get(entity_a);
        Motion &wall_motion = registry.motions.get(entity_b);

        // Keep enemies using AABB behavior for now
        if (registry.enemies.has(entity_a) && !registry.players.has(entity_a)) {
            // existing AABB separation (unchanged)
            vec2 seperation = actor_motion.position - wall_motion.position;
            vec2 wall_half_size = wall_motion.scale / 2.f;
            vec2 actor_half_size = actor_motion.scale / 2.f;

            float overlap_x = (wall_half_size.x + actor_half_size.x) - abs(seperation.x);
            float overlap_y = (wall_half_size.y + actor_half_size.y) - abs(seperation.y);

            if (overlap_x < overlap_y)
                actor_motion.position.x += (seperation.x > 0) ? overlap_x : -overlap_x;
            else
                actor_motion.position.y += (seperation.y > 0) ? overlap_y : -overlap_y;
            return;
        }

        // If the actor has a MeshCollider (player), use SAT-MTV to compute minimal push
        if (MESH_COLLISION_MODE == 0 && registry.meshColliders.has(entity_a) && registry.players.has(entity_a)) {
            MeshCollider &mc = registry.meshColliders.get(entity_a);

            // Build actor polygon in world-space
            if (registry.meshPtrs.has(entity_a)) {
                Mesh *mesh = registry.meshPtrs.get(entity_a);
                float base = mesh->original_size.x; // or .y (use appropriate axis)
                auto polyActorWorld = transformPolygon(mc.vertices, actor_motion, mc.authoring_px);
                // Actor polygon bbox
                vec2 minP = polyActorWorld[0], maxP = polyActorWorld[0];
                for (const vec2 &p : polyActorWorld) {
                    minP.x = std::min(minP.x, p.x);
                    minP.y = std::min(minP.y, p.y);
                    maxP.x = std::max(maxP.x, p.x);
                    maxP.y = std::max(maxP.y, p.y);
                }

                // Wall AABB world polygon
                vec2 half = wall_motion.scale / 2.f;
                std::vector<vec2> aabbPoly = {
                    wall_motion.position + vec2{-half.x, -half.y}, wall_motion.position + vec2{half.x, -half.y},
                    wall_motion.position + vec2{half.x, half.y}, wall_motion.position + vec2{-half.x, half.y}};
                vec2 minW = aabbPoly[0], maxW = aabbPoly[0];
                for (const vec2 &p : aabbPoly) {
                    minW.x = std::min(minW.x, p.x);
                    minW.y = std::min(minW.y, p.y);
                    maxW.x = std::max(maxW.x, p.x);
                    maxW.y = std::max(maxW.y, p.y);
                }

                // Log for diagnostics
                /*spdlog::debug(
                    "COLLISION DEBUG entity={} actor_scale=({}, {}) poly_min=({}, {}) poly_max=({}, {}) wall_min=({},
                   {}) wall_max=({}, {})", entity_a.id(), actor_motion.scale.x, actor_motion.scale.y, minP.x, minP.y,
                   maxP.x, maxP.y, minW.x, minW.y, maxW.x, maxW.y);*/

                // Compute MTV
                vec2 mtv;
                if (computePolygonMTV(polyActorWorld, aabbPoly, mtv)) {
                    // Collision detected — let PhysicsSystem handle motion resolution.
                    spdlog::debug(
                        "CollisionSystem detected mesh-vs-wall for entity {} mtv=({}, {}); physics will resolve",
                        entity_a.id(), mtv.x, mtv.y);
                    // Do NOT modify actor_motion.position or actor_motion.velocity here.
                } else {
                    spdlog::debug("No MTV (no intersection) for entity {}", entity_a.id());
                }
            } else {
                auto polyActorWorld = transformPolygon(mc.vertices, actor_motion, /*fallback base*/ 128.0f);
                // Actor polygon bbox
                vec2 minP = polyActorWorld[0], maxP = polyActorWorld[0];
                for (const vec2 &p : polyActorWorld) {
                    minP.x = std::min(minP.x, p.x);
                    minP.y = std::min(minP.y, p.y);
                    maxP.x = std::max(maxP.x, p.x);
                    maxP.y = std::max(maxP.y, p.y);
                }

                // Wall AABB world polygon
                vec2 half = wall_motion.scale / 2.f;
                std::vector<vec2> aabbPoly = {
                    wall_motion.position + vec2{-half.x, -half.y}, wall_motion.position + vec2{half.x, -half.y},
                    wall_motion.position + vec2{half.x, half.y}, wall_motion.position + vec2{-half.x, half.y}};
                vec2 minW = aabbPoly[0], maxW = aabbPoly[0];
                for (const vec2 &p : aabbPoly) {
                    minW.x = std::min(minW.x, p.x);
                    minW.y = std::min(minW.y, p.y);
                    maxW.x = std::max(maxW.x, p.x);
                    maxW.y = std::max(maxW.y, p.y);
                }

                // log for diagnostics
                /* spdlog::debug(
                    "COLLISION DEBUG entity={} actor_scale=({}, {}) poly_min=({}, {}) poly_max=({}, {}) wall_min=({},
                   {}) wall_max=({}, {})", entity_a.id(), actor_motion.scale.x, actor_motion.scale.y, minP.x, minP.y,
                   maxP.x, maxP.y, minW.x, minW.y, maxW.x, maxW.y);
                    */
                // Compute MTV
                vec2 mtv;
                if (computePolygonMTV(polyActorWorld, aabbPoly, mtv)) {
                    // Collision detected — let PhysicsSystem handle motion resolution.
                    spdlog::debug(
                        "CollisionSystem detected mesh-vs-wall for entity {} mtv=({}, {}); physics will resolve",
                        entity_a.id(), mtv.x, mtv.y);
                    // Do NOT modify actor_motion.position or actor_motion.velocity here.
                } else {
                    spdlog::debug("No MTV (no intersection) for entity {}", entity_a.id());
                }
            }
            return;
        }

        // Default fallback: existing AABB separation (covers other actor cases and players without mesh)
        vec2 seperation = actor_motion.position - wall_motion.position;
        vec2 wall_half_size = wall_motion.scale / 2.f;
        vec2 actor_half_size = actor_motion.scale / 2.f;

        float overlap_x = (wall_half_size.x + actor_half_size.x) - abs(seperation.x);
        float overlap_y = (wall_half_size.y + actor_half_size.y) - abs(seperation.y);

        if (overlap_x < overlap_y) {
            if (seperation.x > 0)
                actor_motion.position.x += overlap_x;
            else
                actor_motion.position.x -= overlap_x;
        } else {
            if (seperation.y > 0)
                actor_motion.position.y += overlap_y;
            else
                actor_motion.position.y -= overlap_y;
        }
    }
}

void CollisionSystem::handle_actor_actor(Entity entity_a, Entity entity_b) {
    if (is_actor(entity_a) && (is_actor(entity_b))) {
        Motion &motion_a = registry.motions.get(entity_a);
        Motion &motion_b = registry.motions.get(entity_b);

        vec2 separation = motion_a.position - motion_b.position;
        float radius_a = motion_a.scale.x / 2.f;
        float radius_b = motion_b.scale.x / 2.f;

        float overlap = (radius_a + radius_b) - length(separation);
        if (overlap > 0.f) {
            float tolerance = 0.01f;
            float correction_mag = max(overlap - tolerance, 0.f) / 2.f;
            vec2 correction = normalize(separation) * correction_mag;

            motion_a.position += correction;
            motion_b.position -= correction;
        }
    }
}

void CollisionSystem::handle_bubbler_collision(Entity entity_a, Entity entity_b, float &elapsed_ms) {
    if (registry.players.has(entity_a) && registry.bubblers.has(entity_b)) {
        if (registry.fireControls.has(entity_a)) {
            FireControl &fc = registry.fireControls.get(entity_a);
            Bubbler &bubbler = registry.bubblers.get(entity_b);

            fc.refill_progress = fc.refill_progress + elapsed_ms * bubbler.ammo_per_second;
            // Ammo cannot exceed max ammo
            int new_ammo = min((int)(fc.refill_progress / PROGRESS_PER_AMMO), fc.max_ammo - fc.ammo);
            fc.ammo += new_ammo;
            // Refill progress cannot exceed PROGRESS_PER_AMMO
            fc.refill_progress = min(fc.refill_progress - new_ammo * PROGRESS_PER_AMMO, (float)PROGRESS_PER_AMMO);
        }
    }
};

void CollisionSystem::handle_tutorial_collision(Entity entity_a, Entity entity_b) {
    if (registry.players.has(entity_a) && registry.tutorialTriggers.has(entity_b)) {
        TutorialTrigger &tt = registry.tutorialTriggers.get(entity_b);

        Progress &progress = registry.progresses.components[0];
        progress.is_paused = true;
        progress.is_tutorial_step_called = true;
        progress.tutorial_step_id = tt.id;

        spdlog::debug("Triggered tutorial {}", (int)progress.tutorial_step_id);
        registry.remove_all_components_of(entity_b);
    }
}
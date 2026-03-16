// internal
#include "systems/physics/physics_system.hpp"
#include "systems/physics/mesh_collision.hpp"
#include "systems/world/world_init.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <spdlog/spdlog.h>

// Toggle for mesh collider, 0 = on.
static int MESH_COLLISION_MODE = 1; // set to any other value to turn it off and us AABB
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion) {
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
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
bool collision_AABB(const Motion &motion1, const Motion &motion2) {
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

// collides with wall: if the entity has a MeshCollider use polygon vs AABB SAT test,
// otherwise fall back to the fast AABB test.
bool collides_with_wall(Entity entity, const Motion &entity_motion) {
    // Loop through all wall entities
    for (uint i = 0; i < registry.walls.entities.size(); i++) {
        Entity wall_entity = registry.walls.entities[i];
        if (!registry.motions.has(wall_entity))
            continue;

        Motion &wall_motion = registry.motions.get(wall_entity);

        // If this moving entity has a mesh collider, do a narrow-phase polygon vs AABB test
        if (registry.meshColliders.has(entity)) {
            MeshCollider &mc = registry.meshColliders.get(entity);

            // Use the MeshCollider's stored authoring_px so transforms match collision system
            float sprite_base = mc.authoring_px > 0.f ? mc.authoring_px : 128.0f;

            // build polygon in world-space using same math as collision system
            auto polyWorld = transformPolygon(mc.vertices, entity_motion, sprite_base);

            // build wall AABB polygon
            vec2 half = wall_motion.scale / 2.f;
            std::vector<vec2> aabbPoly = {
                wall_motion.position + vec2{-half.x, -half.y}, wall_motion.position + vec2{half.x, -half.y},
                wall_motion.position + vec2{half.x, half.y}, wall_motion.position + vec2{-half.x, half.y}};

            // use the exact SAT MTV test (presence of MTV => intersection)
            vec2 tmpMtv;
            if (computePolygonMTV(polyWorld, aabbPoly, tmpMtv)) {
                return true;
            }
        } else {
            // fallback fast-box test
            if (collision_AABB(entity_motion, wall_motion)) {
                return true;
            }
        }
    }

    return false;
}

void PhysicsSystem::step(float elapsed_ms) {
    // Move each entity that has motion
    auto &motion_registry = registry.motions;
    for (uint i = 0; i < motion_registry.components.size(); i++) // changed so i can use index to get entity, sorry
    {
        Motion &motion = motion_registry.components[i];
        Entity entity = motion_registry.entities[i];
        vec2 old_position = motion.position;

        // ensure a sane last_safe_position to avoid teleporting to {0,0}
        const float SAFE_POS_EPS = 1e-3f;
        if (length(motion.last_safe_position) < SAFE_POS_EPS)
            motion.last_safe_position = motion.position;

        auto velocity = motion.velocity * elapsed_ms / 1000.f;

        // Adjust angle based on angular velocity
        motion.angle += motion.angular_velocity * elapsed_ms / 1000.f;
        auto angle = motion.angle;

        // TODO: Optimize this nested loop
        // Apply SpeedMultiplier power up
        for (Entity &sm_entity : registry.speedMultipliers.entities) {
            if (registry.targets.has(sm_entity)) {
                Target &t = registry.targets.get(sm_entity);
                if (t.target == entity) {
                    SpeedMultiplier &sm = registry.speedMultipliers.get(sm_entity);
                    velocity *= sm.multiplier;
                }
            }
        }

        float vx_prime = cos(angle) * velocity.x - sin(angle) * velocity.y;
        float vy_prime = sin(angle) * velocity.x + cos(angle) * velocity.y;

        vec2 movement = vec2(vx_prime, vy_prime);

        // substep movement to avoid tunneling and ensure stable contact handling.
        const float MAX_STEP_PX = 6.0f; // tune: 3-8 px
        int substeps = std::max(1, (int)std::ceil(length(movement) / MAX_STEP_PX));
        vec2 stepMove = movement / float(substeps);

        // contact persistence
        const int CONTACT_PERSIST_FRAMES = 6;
        bool contact_happened = false;

        // Per-substep resolution: apply step, then resolve collisions immediately.
        // This prevents deep penetration and reduces jitter from repeated large corrections.
        for (int s = 0; s < substeps; ++s) {
            motion.position += stepMove;

            // only actors need collision blocking here (resolve per substep)
            if (!(registry.players.has(entity) || registry.enemies.has(entity)))
                continue;

            bool substepHadPenetration = false;

            // mesh-collider actors (players) use SAT vs wall AABBs but resolved per-substep.
            if (registry.meshColliders.has(entity) && registry.players.has(entity) && MESH_COLLISION_MODE == 0) {
                MeshCollider &mc = registry.meshColliders.get(entity);
                float sprite_base = mc.authoring_px > 0.f ? mc.authoring_px : 128.0f;

                // iteratively resolve small penetrations for this substep only
                const int MAX_RESOLVE_ITER = 4;
                const float MIN_MTV_TO_APPLY = 0.45f; // slightly higher to ignore micro-noise
                const float MAX_TRANSLATE_FACTOR = 2.0f;
                const float NUDGE_PX = 0.04f; // extremely small nudge only

                // capture forward before resolution so we can preserve it on fallback
                float prev_local_forward = motion.velocity.x;

                // saved computed local velocity so fallback code can access it outside the iter block
                float saved_computed_local_x = 0.f;
                float saved_computed_local_y = 0.f;

                for (int iter = 0; iter < MAX_RESOLVE_ITER; ++iter) {
                    auto polyWorld = transformPolygon(mc.vertices, motion, sprite_base);

                    std::vector<vec2> perWallMTVs;
                    perWallMTVs.reserve(registry.walls.entities.size());
                    for (uint wi = 0; wi < registry.walls.entities.size(); ++wi) {
                        Entity wall_entity = registry.walls.entities[wi];
                        if (!registry.motions.has(wall_entity))
                            continue;
                        Motion &wall_motion = registry.motions.get(wall_entity);

                        // build the wall AABB polygon in world space
                        vec2 halfW = wall_motion.scale * 0.5f;
                        std::vector<vec2> aabbPoly = {wall_motion.position + vec2{-halfW.x, -halfW.y},
                                                      wall_motion.position + vec2{halfW.x, -halfW.y},
                                                      wall_motion.position + vec2{halfW.x, halfW.y},
                                                      wall_motion.position + vec2{-halfW.x, halfW.y}};

                        // use the exact SAT MTV test (presence of MTV => intersection)
                        vec2 tmpMtv;
                        if (computePolygonMTV(polyWorld, aabbPoly, tmpMtv)) {
                            perWallMTVs.push_back(tmpMtv);
                        }
                    } // walls loop

                    if (perWallMTVs.empty())
                        break; // nothing to resolve

                    // ff there's only one wall, use its MTV. For multiple walls compute a
                    // weighted average normal (normals weighted by overlap) and apply a
                    // displacement along that averaged normal by the largest overlap. This
                    // tends to produce a diagonal correction at corners that clears both.
                    vec2 combined = {0.f, 0.f};
                    if (perWallMTVs.size() == 1) {
                        combined = perWallMTVs[0];
                    } else {
                        float max_overlap = 0.f;
                        vec2 weighted = {0.f, 0.f};
                        for (const vec2 &m : perWallMTVs) {
                            float l = length(m);
                            if (l <= 0.f)
                                continue;
                            vec2 n = m / l;
                            weighted += n * l; // weight by overlap
                            if (l > max_overlap)
                                max_overlap = l;
                        }

                        float wlen = length(weighted);
                        const float EPS_DIR = 1e-6f;
                        if (wlen > EPS_DIR) {
                            vec2 avgn = weighted / wlen;
                            combined = avgn * max_overlap;
                        } else {
                            // fallback: use mean MTV (shouldn't really happen tbh)
                            vec2 sum = {0.f, 0.f};
                            for (const vec2 &m : perWallMTVs)
                                sum += m;
                            combined = sum / float(perWallMTVs.size());
                        }
                    }
                    float combined_len = length(combined);

                    spdlog::debug("PHYS_MTV entity={} iter={} combined=({}, {}) len={} perWalls={}", entity.id(), iter,
                                  combined.x, combined.y, combined_len, perWallMTVs.size());

                    // pushes the player further into geometry causing permanent stick.
                    {
                        Motion tmpTest = motion;
                        tmpTest.position += combined;
                        if (collides_with_wall(entity, tmpTest)) {
                            // Try inverting the MTV
                            combined = -combined;
                            tmpTest = motion;
                            tmpTest.position += combined;
                            if (collides_with_wall(entity, tmpTest)) {
                                // As a last resort, amplify the MTV along its direction to attempt separation
                                float clen = length(combined);
                                if (clen > 1e-6f) {
                                    vec2 nd = combined / clen;
                                    combined = nd * (clen + 1.0f);
                                }
                            }
                        }
                        // recompute length after potential changes
                        combined_len = length(combined);
                    }

                    if (combined_len < MIN_MTV_TO_APPLY) {
                        // treat as resolved for this substep
                        break;
                    }

                    // cap translation per iteration
                    const float MAX_TRANSLATE = std::max<float>(1.0f, length(stepMove) * MAX_TRANSLATE_FACTOR + 1.0f);
                    if (combined_len > MAX_TRANSLATE)
                        combined = (combined / combined_len) * MAX_TRANSLATE;

                    // apply combined correction once for this iteration
                    motion.position += combined;
                    substepHadPenetration = true;
                    contact_happened = true; // mark that we had contact this frame

                    // tiny outward nudge only when really tiny overlap remains
                    if (combined_len < NUDGE_PX) {
                        motion.position += normalize(combined) * NUDGE_PX;
                    }

                    // velocity correction in world-space (players store local velocity, so convert)
                    float ca = cos(angle), sa = sin(angle);
                    vec2 world_vel = vec2(ca * motion.velocity.x - sa * motion.velocity.y,
                                          sa * motion.velocity.x + ca * motion.velocity.y);

                    // determine if player is actively pressing forward so we can preserve that channel
                    bool pressing_forward =
                        registry.keyboardInputs.has(entity) &&
                        registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];

                    vec2 normal = normalize(combined);
                    // remove inward component only
                    float vn = dot(world_vel, normal);
                    if (vn < 0.f)
                        world_vel -= normal * vn;

                    // preserve sliding: high tangential retention but zero very small grazing speeds
                    vec2 tangent = vec2(-normal.y, normal.x);
                    float vt = dot(world_vel, tangent);
                    const float TANGENTIAL_DAMPING = 0.98f;
                    vt *= TANGENTIAL_DAMPING;
                    const float ZERO_TANGENT_THRESHOLD = 0.5f; // world units/sec
                    if (std::abs(vt) < ZERO_TANGENT_THRESHOLD)
                        vt = 0.f;

                    world_vel = normal * dot(world_vel, normal) + tangent * vt;

                    // Convert corrected world velocity back to local space
                    float computed_local_x = ca * world_vel.x + sa * world_vel.y;
                    float computed_local_y = -sa * world_vel.x + ca * world_vel.y;

                    // If the player is not pressing forward, remove any lateral/tangential component
                    // to eliminate post-collision drift entirely. Forward channel (local x) is preserved
                    // so the player can continue to drive when they press forward.
                    if (!pressing_forward) {
                        computed_local_y = 0.f;
                    }

                    // store for fallback use
                    saved_computed_local_x = computed_local_x;
                    saved_computed_local_y = computed_local_y;

                    // Preserve player's forward channel when pressing W.
                    const float LATERAL_DAMPING_WHILE_PRESSING = 0.40f; // stronger damping to reduce slide (was 0.88)

                    // Preserve forward momentum when player is holding forward: prefer the previous
                    // forward channel if it has larger magnitude than the computed correction. This
                    // prevents collisions from immediately killing player forward speed.
                    if (pressing_forward) {
                        float use_forward = std::abs(computed_local_x) > std::abs(prev_local_forward)
                                                ? computed_local_x
                                                : prev_local_forward;
                        motion.velocity.x = use_forward;
                        // allow a damped lateral component when the player is pressing forward
                        motion.velocity.y = computed_local_y * LATERAL_DAMPING_WHILE_PRESSING;
                    } else {
                        motion.velocity.x = computed_local_x;
                        // not pressing forward: we've already zeroed computed_local_y above to remove drift
                        motion.velocity.y = 0.f;
                    }

                    // cache contact normal and set cooldown (stabilizes next substeps)
                    motion.contact_normal = normal;
                    motion.contact_cooldown_frames = CONTACT_PERSIST_FRAMES;

                    // loop to re-check (polyWorld rebuilt at top of loop next iteration)
                } // resolve iterations

                // if this substep ended without penetration, update last_safe_position (but DO NOT immediately clear
                // cached normal)
                Motion tmp = motion;
                if (!collides_with_wall(entity, tmp)) {
                    motion.last_safe_position = motion.position;
                    // do not clear contact_normal here; let cooldown handle the expiry
                } else {
                    // contact present this substep, keep cache refreshed above
                    // Failed to resolve penetration fully -> attempt a safe slide + small outward nudge before giving
                    // up.
                    vec2 escape_normal = motion.contact_normal;
                    if (length(escape_normal) < 1e-5f) {
                        // fallback to movement direction if we don't have a reliable normal
                        if (length(stepMove) > 1e-5f)
                            escape_normal = normalize(stepMove);
                        else
                            escape_normal = vec2{0.f, -1.f};
                    }

                    // pick a sane safe_pos
                    vec2 safe_pos = motion.last_safe_position;
                    if (length(safe_pos) < SAFE_POS_EPS)
                        safe_pos = old_position;

                    // First: try to slide along the wall tangent from safe_pos.
                    vec2 tangent = normalize(vec2(-escape_normal.y, escape_normal.x));

                    // Candidate: project the attempted stepMove onto the tangent and apply it
                    float slideAmount = dot(stepMove, tangent);
                    vec2 candidatePos = safe_pos + tangent * slideAmount;

                    // Test candidate slide position (work on a temporary copy)
                    Motion tmpSlide = motion;
                    tmpSlide.position = candidatePos;
                    bool slide_ok = !collides_with_wall(entity, tmpSlide);

                    spdlog::debug("PHYS_FALLBACK entity={} safe=({}, {}) escape=({}, {}) cand=({}, {}) slide_ok={} "
                                  "saved_vel=({}, {}) prev_forward={}",
                                  entity.id(), safe_pos.x, safe_pos.y, escape_normal.x, escape_normal.y, candidatePos.x,
                                  candidatePos.y, slide_ok, saved_computed_local_x, saved_computed_local_y,
                                  prev_local_forward);

                    if (slide_ok) {
                        // slide succeeded: commit slide position and preserve computed local velocity
                        motion.position = candidatePos;
                        // remember this as a safe position so future fallbacks won't revert to a stale safe_pos
                        motion.last_safe_position = motion.position;
                        // preserve computed local velocity only if player is pressing forward; otherwise clear to avoid
                        // drift
                        bool pressing_forward_now =
                            registry.keyboardInputs.has(entity) &&
                            registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];
                        if (pressing_forward_now) {
                            float use_x = std::abs(saved_computed_local_x) > std::abs(prev_local_forward)
                                              ? saved_computed_local_x
                                              : prev_local_forward;
                            motion.velocity.x = use_x;
                            motion.velocity.y = saved_computed_local_y;
                        } else {
                            motion.velocity = {0.f, 0.f};
                        }
                    } else {
                        // try a small outward nudge along the tangent (and slightly outward along normal) to escape
                        // corner.
                        const float OUTWARD_NUDGE = std::max(0.8f, MIN_MTV_TO_APPLY);
                        vec2 nudgePos =
                            safe_pos + tangent * (slideAmount * 0.5f) + normalize(escape_normal) * OUTWARD_NUDGE;

                        Motion tmpNudge = motion;
                        tmpNudge.position = nudgePos;
                        bool nudge_ok = !collides_with_wall(entity, tmpNudge);

                        spdlog::debug("PHYS_NUDGE entity={} nudgePos=({}, {}) nudge_ok={}", entity.id(), nudgePos.x,
                                      nudgePos.y, nudge_ok);

                        if (nudge_ok) {
                            motion.position = nudgePos;
                            // remember nudge as safe spot
                            motion.last_safe_position = motion.position;
                            // preserve computed local velocity only if player is pressing forward; otherwise clear to
                            // avoid drift
                            bool pressing_forward_now =
                                registry.keyboardInputs.has(entity) &&
                                registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];
                            if (pressing_forward_now) {
                                float use_x = std::abs(saved_computed_local_x) > std::abs(prev_local_forward)
                                                  ? saved_computed_local_x
                                                  : prev_local_forward;
                                motion.velocity.x = use_x;
                                motion.velocity.y = saved_computed_local_y;
                            } else {
                                motion.velocity = {0.f, 0.f};
                            }
                        } else {
                            // last resort: revert to safe_pos but preserve tangential/forward velocity so sliding can
                            // continue.
                            motion.position = safe_pos;
                            bool pressing_forward_now =
                                registry.keyboardInputs.has(entity) &&
                                registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];
                            if (pressing_forward_now) {
                                motion.velocity.x = prev_local_forward;
                                // keep motion.velocity.y as-is (do not zero)
                            } else {
                                // preserve any meaningful tangential speed; only clear micro-drift
                                const float MICRO_DRIFT_THRESHOLD = 0.5f;
                                if (std::abs(motion.velocity.x) < MICRO_DRIFT_THRESHOLD &&
                                    std::abs(motion.velocity.y) < MICRO_DRIFT_THRESHOLD) {
                                    motion.velocity = {0.f, 0.f};
                                } else {
                                    const float FALLBACK_DAMPING = 0.98f;
                                    motion.velocity.x *= FALLBACK_DAMPING;
                                    motion.velocity.y *= FALLBACK_DAMPING;
                                }
                            }
                        }
                    }

                    // cache contact normal and set cooldown (stabilizes next substeps)
                    motion.contact_normal = escape_normal;
                    motion.contact_cooldown_frames = CONTACT_PERSIST_FRAMES;
                }
            }
            // simple AABB actor path (enemies or actors without mesh) - axis separation per substep
            else {
                // use simple AABB separation per substep to avoid clipping
                if (collides_with_wall(entity, motion)) {
                    // revert this substep and try axis-separated move
                    motion.position -= stepMove;

                    // try only moving horizontally this substep
                    motion.position.x += stepMove.x;
                    if (collides_with_wall(entity, motion)) {
                        motion.position.x -= stepMove.x; // blocked
                    }

                    // try only moving vertically this substep
                    motion.position.y += stepMove.y;
                    if (collides_with_wall(entity, motion)) {
                        // move blocked; revert vertical
                        motion.position.y -= stepMove.y;
                    }

                    // mark that we had a contact this frame so cooldown refresh happens below
                    contact_happened = true;
                    motion.contact_cooldown_frames = CONTACT_PERSIST_FRAMES;
                } else {
                    // no collision this substep
                    motion.last_safe_position = motion.position;
                    // do not clear contact_normal here; cooldown will expire it
                }
            }
        } // end substeps

        // --- contact cooldown & exit handling ---
        // If we had contact this frame, refresh cooldown. Otherwise decay it and if it expires, run exit cleanup.
        if (contact_happened) {
            motion.contact_cooldown_frames = CONTACT_PERSIST_FRAMES;
        } else if (motion.contact_cooldown_frames > 0) {
            // decay
            motion.contact_cooldown_frames--;
            // if it just expired, run exit cleanup using the stored contact_normal
            if (motion.contact_cooldown_frames == 0 && length(motion.contact_normal) > 1e-6f) {
                vec2 prev_contact = motion.contact_normal;

                // world velocity (player stores local)
                float ca_exit = cos(angle), sa_exit = sin(angle);
                vec2 world_vel = vec2(ca_exit * motion.velocity.x - sa_exit * motion.velocity.y,
                                      sa_exit * motion.velocity.x + ca_exit * motion.velocity.y);

                // tangent from previous contact normal
                vec2 tangent = normalize(vec2(-prev_contact.y, prev_contact.x));
                float vt = dot(world_vel, tangent);

                spdlog::debug("PHYS_EXIT entity={} prev_contact=({}, {}) world_vel=({}, {}) vt={} local_vel=({}, {})",
                              entity.id(), prev_contact.x, prev_contact.y, world_vel.x, world_vel.y, vt,
                              motion.velocity.x, motion.velocity.y);

                // decide whether to clear lateral tangential motion on exit.
                // preserve the player's forward local channel so they can "drive by" the wall.
                bool pressing_forward_exit =
                    registry.keyboardInputs.has(entity) &&
                    registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];

                const float RELEASE_TANGENT_THRESHOLD = 2.0f; // lower so meaningful tangential speed survives
                // clear lateral/tangential motion on exit to avoid drift flipping when the wall ends.
                // preserve forward channel only if player is actively pressing forward; otherwise stop.
                if (!pressing_forward_exit) {
                    motion.velocity = {0.f, 0.f};
                } else {
                    // keep forward (local x) as-is but remove any lateral drift
                    motion.velocity.y = 0.f;
                    // motion.velocity.x remains unchanged so forward input still applies
                }

                // clear contact cache now that exit cleanup is done
                motion.contact_normal = vec2{0.f, 0.f};

                // final clamp to avoid micro-drift - only clamp when player not driving forward
                const float MIN_VEL_TO_KEEP = 1.0f; // px/s - much smaller to avoid killing input-driven motion
                bool pressing_forward_final =
                    registry.keyboardInputs.has(entity) &&
                    registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];
                // convert local->world to measure final magnitude
                vec2 final_world = vec2(ca_exit * motion.velocity.x - sa_exit * motion.velocity.y,
                                        sa_exit * motion.velocity.x + ca_exit * motion.velocity.y);
                // zero out micro-drift when not actively pressing forward
                float final_world_speed = length(final_world);
                if (!pressing_forward_final && final_world_speed < MIN_VEL_TO_KEEP) {
                    motion.velocity = {0.f, 0.f};
                }
            }
        }

        // Stabilize tiny remaining velocity when not in contact (players only).
        if (registry.players.has(entity) && motion.contact_cooldown_frames == 0) {
            // Convert stored local velocity to world velocity (players store local)
            float ca_clamp = cos(angle), sa_clamp = sin(angle);
            vec2 world_vel = vec2(ca_clamp * motion.velocity.x - sa_clamp * motion.velocity.y,
                                  sa_clamp * motion.velocity.x + ca_clamp * motion.velocity.y);

            // when the player is actively holding MoveForward allow very small speeds to accumulate
            // so the input can ramp the local velocity across frames. When not pressing, clamp micro-drift.
            bool pressing_forward = registry.keyboardInputs.has(entity) &&
                                    registry.keyboardInputs.get(entity).current_actions[KeyboardActions::MoveForward];

            const float GLOBAL_MIN_VEL_WHILE_IDLE = 6.0f;   // px/s - clamp when player not pressing forward
            const float GLOBAL_MIN_VEL_WHILE_MOVING = 0.5f; // px/s - allow accumulation while input held

            float threshold = pressing_forward ? GLOBAL_MIN_VEL_WHILE_MOVING : GLOBAL_MIN_VEL_WHILE_IDLE;
            // clamp micro-drift when idle
            float world_speed = length(world_vel);
            if (!pressing_forward && world_speed < threshold) {
                motion.velocity = {0.f, 0.f};
            }
        }
    }
    // Clamp player to arena bounds
    // Disabled since the walls surrounding the map should be sufficient
    /*
    for (Entity player_entity : registry.players.entities) {
        assert(motion_registry.has(player_entity) && "Player has no motion entity!");
        Motion &player_motion = motion_registry.get(player_entity);
        TileMap &tilemap = registry.tilemaps.components[0];
        vec2 half = get_bounding_box(player_motion) / 2.f;
        vec2 margin = vec2(TILE_SIZE_PX + half.x, TILE_SIZE_PX + half.y);
        float min_x = margin.x;
        float max_x = tilemap.width * TILE_SIZE_PX - margin.x;
        float min_y = margin.y;
        float max_y = tilemap.height * TILE_SIZE_PX - margin.y;

        player_motion.position.x = glm::clamp(player_motion.position.x, min_x, max_x);
        player_motion.position.y = glm::clamp(player_motion.position.y, min_y, max_y);
    }
    */
    // check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
    for (uint i = 0; i < motion_container.components.size(); i++) {
        Motion &motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];

        // note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
        for (uint j = i + 1; j < motion_container.components.size(); j++) {
            Motion &motion_j = motion_container.components[j];
            if (collision_AABB(motion_i, motion_j)) {
                Entity entity_j = motion_container.entities[j];
                // Create a collisions event
                registry.collisions.insert(entity_i, {entity_j}, false);
                registry.collisions.insert(entity_j, {entity_i}, false);
            }
        }
    }
}

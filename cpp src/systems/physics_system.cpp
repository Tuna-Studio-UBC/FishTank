// If entity has a mesh collider, attempt iterative SAT-MTV resolution against walls.
if (registry.meshColliders.has(entity)) {
    MeshCollider &mc = registry.meshColliders.get(entity);

    // Use authoring pixel base stored on the collider
    float sprite_base = mc.authoring_px > 0.f ? mc.authoring_px : 128.0f;

    // Iteratively resolve penetrations against all walls (few iterations)
    const int MAX_RESOLVE_ITER = 6;
    bool resolved = false;

    for (int iter = 0; iter < MAX_RESOLVE_ITER; ++iter) {
        // Build actor polygon in world-space with current motion.position
        auto polyWorld = transformPolygon(mc.vertices, motion, sprite_base);

        bool anyPenetration = false;
        float total_translation_this_iter = 0.0f;

        // Check each wall and resolve all found penetrations in this iteration.
        for (uint wi = 0; wi < registry.wall.entities.size(); ++wi) {
            Entity wall_entity = registry.wall.entities[wi];
            if (!registry.motions.has(wall_entity))
                continue;
            Motion &wall_motion = registry.motions.get(wall_entity);

            // Build wall AABB polygon
            vec2 half = wall_motion.scale / 2.f;
            std::vector<vec2> aabbPoly = {
                wall_motion.position + vec2{-half.x, -half.y},
                wall_motion.position + vec2{ half.x, -half.y},
                wall_motion.position + vec2{ half.x,  half.y},
                wall_motion.position + vec2{-half.x,  half.y}
            };

            vec2 mtv;
            if (computePolygonMTV(polyWorld, aabbPoly, mtv)) {
                // Apply the minimal translation vector
                motion.position += mtv;
                float mtv_len = length(mtv);
                total_translation_this_iter += mtv_len;
                anyPenetration = true;

                // Small outward nudge to avoid grazing numerical sticking
                const float NUDGE_PX = 0.5f;
                if (mtv_len > 1e-6f && mtv_len < NUDGE_PX) {
                    vec2 n = mtv / mtv_len;
                    motion.position += n * NUDGE_PX;
                    total_translation_this_iter += NUDGE_PX;
                }

                // remove inward component of velocity (don't let player keep pushing into wall)
                const float EPS = 1e-6f;
                if (mtv_len > EPS) {
                    vec2 n = mtv / mtv_len; // outward normal
                    float vn = dot(motion.velocity, n);
                    if (vn < 0.f) {
                        motion.velocity -= n * vn; // drop inward component
                    }
                }

                // After applying translation, rebuild polyWorld for remaining walls
                polyWorld = transformPolygon(mc.vertices, motion, sprite_base);
                // continue checking other walls in the same iteration (no break)
            }
        } // end wall loop

        // if nothing overlapped this iteration, we are resolved
        if (!anyPenetration || total_translation_this_iter < 1e-3f) {
            resolved = true;
            break;
        }
    } // end iterations

    if (!resolved) {
        // Robust fallback if SAT iterative resolution could not clear penetration:
        motion.position = old_position;

        // try only moving horizontally
        motion.position.x += movement.x;
        if (collides_with_wall(entity, motion)) {
            motion.position.x = old_position.x;
        }

        // try only moving vertically
        motion.position.y = old_position.y + movement.y;
        if (collides_with_wall(entity, motion)) {
            motion.position.y = old_position.y;
        }
    }
} else {
    // Entities without mesh collider keep existing AABB behavior
    if (collides_with_wall(entity, motion)) {
        // simple fallback: revert to old position and try axis-separated movement
        motion.position = old_position;

        // try only moving horizontally
        motion.position.x += movement.x;
        if (collides_with_wall(entity, motion)) {
            // x movement blocked, revert x
            motion.position.x = old_position.x;
        }

        // try moving only vertically
        motion.position.y = old_position.y + movement.y;
        if (collides_with_wall(entity, motion)) {
            // y movement blocked, revert y
            motion.position.y = old_position.y;
        }
    }
}
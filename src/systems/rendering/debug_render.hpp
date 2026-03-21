#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// Debug rendering utilities
namespace DebugRender {

// Draw a circle using debug line entities
// lifetime_ms: how long the circle should be visible (0 = permanent)
inline void drawCircle(vec2 center, float radius, vec3 color = vec3(1, 0, 0), float lifetime_ms = 0.0f, int segments = 32) {
    const float angleStep = 2.0f * M_PI / segments;
    
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        
        vec2 p1 = center + vec2(cos(angle1), sin(angle1)) * radius;
        vec2 p2 = center + vec2(cos(angle2), sin(angle2)) * radius;
        
        // Create two small entities to represent the line segment
        Entity line = Entity("DebugCircleSegment");
        
        Motion &motion = registry.motions.emplace(line);
        motion.position = (p1 + p2) * 0.5f;  // Midpoint
        motion.angle = atan2(p2.y - p1.y, p2.x - p1.x);
        motion.scale = vec2(length(p2 - p1), 2.0f);  // Length and width
        
        registry.colours.emplace(line, Colour(color));
        registry.renderRequests.insert(line, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
                                             EFFECT_ASSET_ID::EGG,
                                             GEOMETRY_BUFFER_ID::DEBUG_LINE,
                                             RENDER_LAYER::OVERLAYS});
        
        if (lifetime_ms > 0.0f) {
            registry.expirations.emplace(line, lifetime_ms);
        }
    }
}

// Draw explosion radius for all explosives (mines and explosive bullets)
inline void drawExplosionRadii() {
    // Draw radius for deployed mines
    for (Entity e : registry.deployedMines.entities) {
        if (registry.explosives.has(e) && registry.motions.has(e)) {
            Explosive &exp = registry.explosives.get(e);
            Motion &motion = registry.motions.get(e);
            
            vec3 color = vec3(1, 0, 0);  // Red for normal
            if (exp.triggered_by_explosion) {
                color = vec3(1, 0.5, 0);  // Orange for doubled
            }
            
            drawCircle(motion.position, exp.radius, color, 16.7f);  // One frame at 60fps
        }
    }
    
    // Draw radius for explosive bullets
    for (Entity e : registry.bullets.entities) {
        if (registry.explosives.has(e) && registry.motions.has(e)) {
            Bullet &bullet = registry.bullets.get(e);
            if (bullet.type == BulletType::EXPLOSIVE) {
                Explosive &exp = registry.explosives.get(e);
                Motion &motion = registry.motions.get(e);
                
                drawCircle(motion.position, exp.radius, vec3(1, 1, 0), 16.7f);  // Yellow
            }
        }
    }
}

} // namespace DebugRender

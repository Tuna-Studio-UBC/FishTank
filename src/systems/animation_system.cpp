#include "systems/animation_system.hpp"
#include "world_init.hpp"
#include <iostream>

void AnimationSystem::step(float elapsed_ms) {
    for (Entity e : registry.animations.entities) {
        Animation &anim = registry.animations.get(e);

        anim.accumulator += elapsed_ms;

        if (anim.accumulator >= anim.frame_time) {
            anim.accumulator = 0.f;
            anim.current_frame = (anim.current_frame + 1) % anim.frame_count;
        }
    }
}

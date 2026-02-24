#include "particle_system.hpp"
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>

std::mt19937 gen(123);
std::uniform_real_distribution<double> dist(0.0, 2.0 * M_PI);

vec2 random_direction_unit_vector() { return direction_to_unit_vector(dist(gen)); }

void spawn_particle(ParticleEmitterRuntime &rt, ParticleEmitter &pe, Motion &m) {
    rt.particles.push_back({m.position, pe.particle_speed * random_direction_unit_vector(), pe.particle_life_ms});
};

void update_particle_positions(ParticleEmitterRuntime &rt, float elapsed_ms) {
    float dt = (elapsed_ms / 1000.f);
    for (Particle &p : rt.particles) {
        p.life_ms -= elapsed_ms;
        // For now, just move particle in a straight line
        p.position += p.velocity * dt;
    }
}

void erase_expired_particles(ParticleEmitterRuntime &rt) {
    rt.particles.erase(
        std::remove_if(rt.particles.begin(), rt.particles.end(), [](const Particle &p) { return p.life_ms <= 0.f; }),
        rt.particles.end());
}

void ParticleSystem::clear_all_particles() {
    for (Entity e : registry.particleEmitters.entities) {
        runtimes.erase(e);
        registry.remove_all_components_of(e);
    }
}

void ParticleSystem::step(float elapsed_ms) {
    // See if we need to create any new runtimes
    for (Entity e : registry.particleEmitters.entities) {
        if (runtimes.find(e) == runtimes.end()) {
            auto result = runtimes.emplace(e, ParticleEmitterRuntime{});
            ParticleEmitterRuntime &rt = result.first->second;
            ParticleEmitter &pe = registry.particleEmitters.get(e);
            rt.texture = pe.texture;
            rt.standalone = pe.standalone;
            spdlog::info("Created new particle emitter: {}", e.name_id());
        }
    }

    // Now loop through runtimes and update each
    // cpp has some interesting behaviour for normal looping... we need to use iterator as we will erase from dict
    // https://langdev.stackexchange.com/questions/3623/why-does-cs-unordered-maperase-fully-invalidate-iterators-not-even-support
    for (auto it = runtimes.begin(); it != runtimes.end();) {
        auto &e = it->first;
        auto &rt = it->second;

        // Check if this runtime is orphaned (Entity is gone)
        if (!rt.standalone && (!registry.motions.has(e) || !registry.particleEmitters.has(e))) {
            update_particle_positions(rt, elapsed_ms);
            erase_expired_particles(rt);

            if (rt.particles.size() > 0) {
                ++it;
                continue;
            }
            
            spdlog::info("Destroyed orphaned standalone particle emitter: {}", e.name_id());
            it = runtimes.erase(it);
            continue;
        }

        // Not orphaned at this point in time
        ParticleEmitter &pe = registry.particleEmitters.get(e);
        Motion &m = registry.motions.get(e);

        // Only update if not infinite
        if (pe.life_ms != -99.f) {
            pe.life_ms = std::max(pe.life_ms - elapsed_ms, 0.f);
        }

        // Destroy expired particle effects
        if (pe.life_ms != -99.f && pe.life_ms <= 0.f) {
            update_particle_positions(rt, elapsed_ms);
            erase_expired_particles(rt);

            if (rt.particles.size() > 0) {
                ++it;
                continue;
            }

            
            if (pe.standalone) {
                registry.remove_all_components_of(e);
                spdlog::info("Destroyed standalone particle emitter: {}", e.name_id());
            } else {
                spdlog::info("Destroyed connected particle emitter: {}", e.name_id());
            }
            it = runtimes.erase(it);
            continue;
        }

        // At this point, we have a still active emitter

        // Add to our accumulator and only spawn particles when we have enough "saved up" time
        rt.spawn_accumulator_ms += elapsed_ms;

        while (rt.spawn_accumulator_ms >= pe.spawn_interval_ms && rt.particles.size() < pe.max_particles) {
            rt.spawn_accumulator_ms -= pe.spawn_interval_ms;
            spawn_particle(rt, pe, m);
        }

        // Update particle positions & erase dead particles
        update_particle_positions(rt, elapsed_ms);
        erase_expired_particles(rt);
        ++it;
    }
}

ParticleSystem particle_system;

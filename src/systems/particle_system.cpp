#include "particle_system.hpp"
#include <iostream>
#include <random>

std::mt19937 gen(123);
std::uniform_real_distribution<double> dist(0.0, 2.0 * M_PI);

vec2 random_direction_unit_vector() {
  return direction_to_unit_vector(dist(gen));
}

void spawn_particle(ParticleEmitterRuntime &rt, ParticleEmitter &pe) {
  rt.particles.push_back({
    pe.position,
    pe.particle_speed * random_direction_unit_vector(),
    pe.particle_life_ms
  });
};

void update_particle_positions(ParticleEmitterRuntime &rt, float elapsed_ms) {
  float dt = (elapsed_ms / 1000.f);
  for (Particle &p : rt.particles)
  {
    p.life_ms -= elapsed_ms;
    // For now, just move particle in a straight line
    p.position += p.velocity * dt;
  }
}

void erase_expired_particles(ParticleEmitterRuntime &rt) {
  rt.particles.erase(
      std::remove_if(
          rt.particles.begin(),
          rt.particles.end(),
          [](const Particle &p)
          { return p.life_ms <= 0.f; }),
      rt.particles.end());
}

void ParticleSystem::clear_all_particles() {
  for (Entity e : registry.particleEmitters.entities) {
    runtimes.erase(e.id());
    registry.remove_all_components_of(e);
  }
}

void ParticleSystem::step(float elapsed_ms)
{
  for (Entity e : registry.particleEmitters.entities)
  {
    ParticleEmitterRuntime &rt = runtimes[e.id()];
    ParticleEmitter &pe = registry.particleEmitters.get(e);
    pe.life_ms -= elapsed_ms;
    
    // Destroy expired particle effects
    if (pe.life_ms <= 0.f)
    {
      update_particle_positions(rt, elapsed_ms);
      erase_expired_particles(rt);

      if (rt.particles.size() > 0) {
        continue;
      }

      std::cout << "Destroyed particle emitter: " << e.name_id() << std::endl;
      runtimes.erase(e.id());
      registry.remove_all_components_of(e);
      continue;
    }

    // Add to our accumulator and only spawn particles when we have enough "saved up" time
    rt.spawn_accumulator_ms += elapsed_ms;

    while (rt.spawn_accumulator_ms >= pe.spawn_interval_ms &&
           rt.particles.size() < pe.max_particles)
    {
      rt.spawn_accumulator_ms -= pe.spawn_interval_ms;
      spawn_particle(rt, pe);
    }

    // Update particle positions
    update_particle_positions(rt, elapsed_ms);

    // Finally, erase dead particles
    erase_expired_particles(rt);
  }
}

ParticleSystem particle_system;
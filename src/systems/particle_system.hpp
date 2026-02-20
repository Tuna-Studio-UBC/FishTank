#pragma once

#include <iostream>

#include "../common.hpp"
#include "../world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Particle struct for information pertaining to each individual particle
struct Particle {
    vec2 position;
    vec2 velocity; // px / s
    float life_ms;
};

struct ParticleEmitterRuntime {
    std::vector<Particle> particles;
    float spawn_accumulator_ms; // Interesting addition to prevent FPS dependent spawn rates
};

class ParticleSystem {
  public:
    // Store separately from ECS and lazy load for better performance
    std::unordered_map<unsigned int, ParticleEmitterRuntime> runtimes;

    void clear_all_particles();

    void step(float elapsed_ms);

    ParticleSystem() {}
};

extern ParticleSystem particle_system;

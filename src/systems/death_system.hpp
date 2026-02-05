#pragma once

#include "../common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

// Updates player and enemy health
class DeathSystem
{
public:
  void step(float elapsed_ms);

  DeathSystem() {}
};

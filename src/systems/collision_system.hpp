#pragma once

#include <iostream>

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "../common.hpp"

class CollisionSystem
{
public:
  void step(float elapsed_ms);

  CollisionSystem() {}

private:
  void handle_bullets(Entity entity_a, Entity entity_b);
};
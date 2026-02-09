#pragma once

#include <iostream>

#include "../common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class CollisionSystem {
  public:
    void step(float elapsed_ms);

    CollisionSystem() {}

  private:
    void handle_walls(Entity entity_a, Entity entity_b);
    void handle_entities_vs_walls(Entity entity_a, Entity entity_b);
    void handle_bullets(Entity bullet_entity, Entity other_entity);
    void handle_hazards(Entity hazard_entity, Entity other_entity);
};
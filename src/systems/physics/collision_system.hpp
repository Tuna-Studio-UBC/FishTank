#pragma once

#include <iostream>

#include <spdlog/spdlog.h>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class CollisionSystem {
  public:
    void step(float elapsed_ms);

    CollisionSystem() {}

  private:
    bool is_actor(Entity entity);

    void handle_walls(Entity entity_a, Entity entity_b);
    void handle_apply_damage(Bullet &bullet, Entity other_entity);
    void handle_bullets(Entity bullet_entity, Entity other_entity);
    void handle_hazards(Entity hazard_entity, Entity other_entity);
    void handle_actors_vs_walls(Entity entity_a, Entity entity_b);
    void handle_actor_actor(Entity entity_a, Entity entity_b);
    void handle_bubbler_collision(Entity entity_a, Entity entity_b, float &elapsed_ms);
    void handle_tutorial_collision(Entity entity_a, Entity entity_b);
};

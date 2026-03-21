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
    void handle_explosives(Entity explosive_entity, vec2 explosion_pos, float radius, float damage);
    void collect_chain_mines(Entity explosive_entity, vec2 center, float radius, std::vector<Entity>& collected_mines, float& total_damage, float& max_radius);
    void handle_hazards(Entity hazard_entity, Entity other_entity);
    void handle_actors_vs_walls(Entity entity_a, Entity entity_b);
    void handle_actor_actor(Entity entity_a, Entity entity_b);
    void handle_bubbler_collision(Entity entity_a, Entity entity_b, float &elapsed_ms);
    void handle_tutorial_collision(Entity entity_a, Entity entity_b);
};

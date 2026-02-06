#include "collision_system.hpp"
#include "../world_init.hpp"

// Compute collisions between entities
void CollisionSystem::step(float elapsed_ms)
{

  ComponentContainer<Collision> &collision_registry = registry.collisions;
  for (uint i = 0; i < collision_registry.components.size(); i++)
  {
    Entity entity_a = collision_registry.entities[i];
    Entity entity_b = collision_registry.components[i].other;

    handle_bullets(entity_a, entity_b);
    handle_hazards(entity_a, entity_b);
  }

  // Clear all collisions for the current step
  registry.collisions.clear();
}

void CollisionSystem::handle_bullets(Entity bullet_entity, Entity other_entity)
{
  if (registry.bullet.has(bullet_entity))
  {
    // Bullet hit player
    if (registry.players.has(other_entity))
    {
      Bullet &bullet = registry.bullet.get(bullet_entity);
      if (bullet.owner != other_entity)
      {
        registry.damage.emplace(other_entity, bullet.damage);
        registry.remove_all_components_of(bullet_entity);
      }
    }
    // Bullet hit enemy
    else if (registry.enemies.has(other_entity))
    {
      Bullet &bullet = registry.bullet.get(bullet_entity);
      if (bullet.owner != other_entity && registry.health.has(other_entity))
      {
        registry.damage.emplace(other_entity, bullet.damage);
        registry.remove_all_components_of(bullet_entity);
      }
    }
    // Bullet hit bullet
    else if (registry.bullet.has(other_entity))
    {
      registry.remove_all_components_of(bullet_entity);
      registry.remove_all_components_of(other_entity);
    }
  }
}

// TODO: Handle applying hazards FX
void CollisionSystem::handle_hazards(Entity hazard_entity, Entity other_entity)
{
  if (registry.hazard.has(hazard_entity))
  {
    Hazard &hazard = registry.hazard.get(hazard_entity);
    auto &config = HAZARD_CONFIGS[(int)hazard.type];

    // Handle hazards against player
    if (registry.players.has(other_entity))
    {
      if (config.duration_ms > 0.f && config.interval_ms > 0.f)
      {
        createOverTimeEffect<Damage>(other_entity, config.interval_ms, config.duration_ms, config.amount);
        registry.remove_all_components_of(hazard_entity);
      }
      else
      {
        registry.damage.emplace(other_entity, config.amount);
        registry.remove_all_components_of(hazard_entity);
      }
    }
    // Handle hazards against enemy
    else if (registry.enemies.has(other_entity))
    {
      registry.damage.emplace(other_entity, config.amount);
      registry.remove_all_components_of(hazard_entity);
    }
  }
}

#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "utilities/parse_tiled.hpp"
#include "configs/enemy_config.hpp"
#include "configs/attack_pattern.hpp"
#include "configs/power_up_config.hpp"

// TODO: create entity functions
// Initiate tile map
void init_tiles();

// Create player entity
Entity createPlayer(RenderSystem *renderer, vec2 position, int health, int max_health);
Entity createEnemy(RenderSystem* renderer, vec2 position, ENEMY_TYPE type);

Entity createBell(RenderSystem* renderer, vec2 position, float interact_radius_px);

Entity createProjectile(vec2 position, float angle, ATTACK_PATTERN attack_pattern, Entity owner);

Entity createPowerUp(RenderSystem *renderer, vec2 position, POWER_UP_TYPE type);

Entity createWall(vec2 position, vec2 scale);

// Component can be: Damage, Heal
template <typename Component, typename... Args>
Entity createOverTimeEffect(Entity target_entity, float interval_ms, float duration_ms, Args &&...args)
{
  auto ot_entity = Entity();
  registry.get<Component>().emplace(ot_entity, std::forward<Args>(args)...);
  registry.overTimeEffect.emplace(ot_entity, interval_ms, duration_ms);
  registry.target.emplace(ot_entity, target_entity);
  return ot_entity;
}

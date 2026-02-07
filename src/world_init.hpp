#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "utilities/parse_tiled.hpp"
#include "configs/enemy_config.hpp"
#include "configs/attack_pattern.hpp"
#include "configs/hazard_config.hpp"
#include "configs/power_up_config.hpp"

// Initiate tile map
void init_tiles();
// create tile walls
void init_tile_walls();
// Create player entity
Entity createPlayer(RenderSystem *renderer, vec2 position);
Entity createEnemy(RenderSystem* renderer, vec2 position, ENEMY_TYPE type);

Entity createBell(RenderSystem* renderer, vec2 position, float interact_radius_px);

Entity createProjectile(vec2 position, float angle, ATTACK_PATTERN attack_pattern, Entity owner);
Entity createMine(RenderSystem *renderer, vec2 position);
Entity createAnemone(RenderSystem *renderer, vec2 position);

Entity createPowerUp(RenderSystem *renderer, vec2 position, POWER_UP_TYPE type);

Entity createWall(const vec2& position, const vec2& scale = vec2(TILE_SIZE_PX));

Entity createCamera(vec2 position);

// Component can be: DamageMultiplier, SpeedMultiplier
template <typename Component, typename... Args>
Entity createExpirationEffect(Entity target_entity, float lifespan_ms, Args &&...args)
{
  auto exp_entity = Entity("ExpirationEffect");
  registry.get<Component>().emplace(exp_entity, std::forward<Args>(args)...);
  registry.expiration.emplace(exp_entity, lifespan_ms);
  registry.target.emplace(exp_entity, target_entity);
  return exp_entity;
}

// Component can be: Damage, Heal
template <typename Component, typename... Args>
Entity createOverTimeEffect(Entity target_entity, float interval_ms, float duration_ms, Args &&...args)
{
  auto ot_entity = Entity("OverTimeEffect");
  registry.get<Component>().emplace(ot_entity, std::forward<Args>(args)...);
  registry.overTimeEffect.emplace(ot_entity, interval_ms, duration_ms);
  registry.target.emplace(ot_entity, target_entity);
  return ot_entity;
}

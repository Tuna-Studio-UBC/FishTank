#pragma once

#include "common.hpp"
#include "configs/attack_pattern.hpp"
#include "configs/enemy_config.hpp"
#include "configs/hazard_config.hpp"
#include "configs/power_up_config.hpp"
#include "systems/rendering/render_system.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "utilities/parse_tiled.hpp"
#include "utility"
extern ECSRegistry registry;

// Create player entity
Entity createPlayer(RenderSystem *renderer, vec2 position);

Entity createEnemy(RenderSystem *renderer, vec2 position, ENEMY_TYPE type);
Entity createLeader(RenderSystem *renderer, vec2 position);
Entity createFollower(RenderSystem *renderer, vec2 position, Entity leader);

Entity createTurret(RenderSystem *renderer, vec2 position);

Entity createBell(RenderSystem *renderer, vec2 position, float interact_radius_px);

Entity createProjectile(vec2 position, float angle, ATTACK_PATTERN attack_pattern, Entity owner);
Entity createMine(RenderSystem *renderer, vec2 position);
Entity createAnemone(RenderSystem *renderer, vec2 position);

Entity createPowerUp(RenderSystem *renderer, vec2 position, POWER_UP_TYPE type);
Entity createBubbler(RenderSystem *renderer, vec2 position);

Entity createWall(const vec2 &position, const vec2 &scale = vec2(TILE_SIZE_PX));

Entity createText(std::string text, vec2 position, float scale, const glm::vec3 &color = glm::vec3(0.f, 0.f, 0.f),
                  const glm::mat4 &trans = glm::mat4(1.f));

Entity createTutorialTrigger(const vec2 position, const vec2 size, const Tutorial id);

Entity createCamera(vec2 position);

// These effects are temporary and standalone
Entity createEffectSmoke(const vec2 position);
Entity createEffectBubbles(const vec2 position);

// Component can be: DamageMultiplier, SpeedMultiplier
template <typename Component, typename... Args>
Entity createExpirationEffect(Entity target_entity, float lifespan_ms, Args &&...args) {
    auto exp_entity = Entity("ExpirationEffect");
    registry.get<Component>().emplace(exp_entity, std::forward<Args>(args)...);
    registry.expirations.emplace(exp_entity, lifespan_ms);
    registry.targets.emplace(exp_entity, target_entity);
    return exp_entity;
}

// Component can be: Damage, Heal
template <typename Component, typename... Args>
Entity createOverTimeEffect(Entity target_entity, float interval_ms, float duration_ms, bool immediate_apply = true,
                            Args &&...args) {
    auto ot_entity = Entity("OverTimeEffect");
    registry.get<Component>().emplace(ot_entity, std::forward<Args>(args)...);
    if (immediate_apply)
        registry.overTimeEffects.emplace(ot_entity, interval_ms, duration_ms, interval_ms - 1);
    else
        registry.overTimeEffects.emplace(ot_entity, interval_ms, duration_ms);
    registry.targets.emplace(ot_entity, target_entity);
    return ot_entity;
}

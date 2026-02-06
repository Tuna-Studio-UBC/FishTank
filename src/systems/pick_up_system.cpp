#include "pick_up_system.hpp"

void PickUpSystem::step(float elapsed_ms)
{
  for (Entity e : registry.pickUp.entities)
  {
    Entity player = registry.players.entities[0];
    vec2 player_pos = registry.motions.get(player).position;

    if (registry.motions.has(e))
    {
      Motion &motion = registry.motions.get(e);
      if (player_in_radius(player_pos, motion, 0.f))
      {
        on_pickup(e, player);
      }
    }
  }
}

void PickUpSystem::on_pickup(Entity pickUp, Entity player)
{
  if (registry.powerUp.has(pickUp))
  {
    PowerUp powerUp = registry.powerUp.get(pickUp);
    auto &config = POWER_UP_CONFIGS[(int)powerUp.type];

    std::cout << "Picked up power up type: " << (int)powerUp.type << std::endl;

    // TODO: add powerups
    switch (powerUp.type)
    {
    case POWER_UP_TYPE::HEAL:
      createOverTimeEffect<Heal>(player, config.duration_ms, config.duration_ms, config.amount);
      break;
    case POWER_UP_TYPE::SPEED_MULTIPLIER:
      // createExpires<SpeedMultiplier>(player, config.duration_ms, config.amount);
      break;
    case POWER_UP_TYPE::DAMAGE_MULTIPLIER:
      // createExpires<DamageMultiplier>(player, config.duration_ms, config.amount);
      break;
    default:
      std::cout << "Unknown power up type" << std::endl;
      break;
    }
  }

  registry.remove_all_components_of(pickUp);
}

// Taken from ai_system
// Probably good enough instead of using AABB collision in the physics system
bool PickUpSystem::player_in_radius(vec2 player_pos, Motion &other_motion, float radius)
{
  vec2 d = player_pos - other_motion.position;
  float true_radius = radius + std::max(other_motion.scale.x, other_motion.scale.y);
  float dist2 = d.x * d.x + d.y * d.y;
  return dist2 <= true_radius * true_radius;
}

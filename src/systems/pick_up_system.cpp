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
      if (entity_in_radius(player_pos - motion.position, motion.scale, 0.f))
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

    std::cout << "Picked up " << pickUp.name_id() << " of type: " << (int)powerUp.type << std::endl;

    // TODO: Add more power ups!!!
    switch (powerUp.type)
    {
    case POWER_UP_TYPE::HEAL:
      createOverTimeEffect<Heal>(player, config.duration_ms, config.duration_ms, config.amount);
      break;
    case POWER_UP_TYPE::SPEED_MULTIPLIER:
      createExpirationEffect<SpeedMultiplier>(player, config.duration_ms, config.amount);
      break;
    case POWER_UP_TYPE::DAMAGE_MULTIPLIER:
      createExpirationEffect<DamageMultiplier>(player, config.duration_ms, config.amount);
      break;
    default:
      std::cout << "Unknown power up type" << std::endl;
      break;
    }
  }

  registry.remove_all_components_of(pickUp);
}

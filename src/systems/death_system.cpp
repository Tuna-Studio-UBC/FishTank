#include <iostream>

#include "death_system.hpp"

void DeathSystem::step(float elapsed_ms)
{
  for (Entity e : registry.health.entities)
  {
    if (registry.dead.has(e))
      continue;

    if (registry.health.has(e))
    {
      Health &h = registry.health.get(e);
      if (h.hp <= 0 && h.max_hp > 0)
      {
        std::cout << e.name_id() << " has died" << std::endl;
        registry.dead.emplace(e);

        if (registry.players.has(e))
        {
          // TODO: Handle player death

        }
        else if (registry.enemies.has(e))
        {
          // TODO: Handle enemy death

          // Drop crackers
          if (registry.dropsCrackers.has(e))
          {
            DropsCrackers &drops = registry.dropsCrackers.get(e);
            Entity player = registry.players.entities[0];
            Wallet &wallet = registry.wallet.get(player);
            wallet.crackers += drops.cracker;

            std::cout << player.name_id() << " collected " << drops.cracker << " crackers! Total: " << wallet.crackers << std::endl;
          }

          registry.remove_all_components_of(e);
        }
      }
    }
  }
}
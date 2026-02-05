#include "heal_damage_system.hpp"

void HealDamageSystem::step(float elapsed_ms)
{
  (void)elapsed_ms;

  handle_damaging();
  handle_healing();
}

void HealDamageSystem::handle_damaging()
{
  for (Entity e : registry.damage.entities)
  {
    if (registry.health.has(e))
    {
      Damage &d = registry.damage.get(e);
      Health &h = registry.health.get(e);
      h.hp = std::max(h.hp - d.amount, 0);
      registry.damage.remove(e);

      // TODO: Handle visual indicators

      printf("Entity %d took %d damage and now has %d/%d HP\n", e.id(), d.amount, h.hp, h.max_hp);
    }
  }
}

void HealDamageSystem::handle_healing()
{
  for (Entity e : registry.heal.entities)
  {
    if (registry.health.has(e))
    {
      Heal &h = registry.heal.get(e);
      Health &he = registry.health.get(e);
      he.hp = std::min(he.hp + h.amount, he.max_hp);
      registry.heal.remove(e);

      // TODO: Handle visual indicators

      printf("Entity %d took %d healing and now has %d/%d HP\n", e.id(), h.amount, he.hp, he.max_hp);
    }
  }
}
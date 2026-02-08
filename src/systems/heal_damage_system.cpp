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

      std::cout << e.name_id() << " took " << d.amount << " damage and now has " << h.hp << "/" << h.max_hp << " HP" << std::endl;
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

      std::cout << e.name_id() << " took " << h.amount << " healing and now has " << he.hp << "/" << he.max_hp << " HP" << std::endl;
    }
  }
}

// TODO: refactor to generic duplicate check if needed
void HealDamageSystem::add_damage(Entity entity, int amount)
{
  if (!registry.damage.has(entity))
  {
    registry.damage.emplace(entity, amount);
  }
  else
  {
    std::cout << "DUPLICATE DAMAGE WAS CALLED" << std::endl;
    Damage &d = registry.damage.get(entity);
    d.amount += amount;
  }
}

void HealDamageSystem::add_heal(Entity entity, int amount)
{
  if (!registry.heal.has(entity))
  {
    registry.heal.emplace(entity, amount);
  }
  else
  {
    Heal &h = registry.heal.get(entity);
    h.amount += amount;
  }
}

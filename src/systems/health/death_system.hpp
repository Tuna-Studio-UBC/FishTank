#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "configs/enemy_config.hpp"

// Updates player and enemy health
class DeathSystem {
  private:
    void changeEnemyType(Entity enemy, ENEMY_TYPE type);

  public:
    void step(float elapsed_ms);

    DeathSystem() {}
};

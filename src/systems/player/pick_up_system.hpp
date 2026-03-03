#pragma once

#include <iostream>

#include "common.hpp"
#include "configs/power_up_config.hpp"
#include "systems/world/world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class PickUpSystem {
  public:
    void step(float elapsed_ms);

    PickUpSystem() {}

  private:
    void on_pickup(Entity pickup, Entity player);
};

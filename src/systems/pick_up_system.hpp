#pragma once

#include <iostream>

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "configs/power_up_config.hpp"
#include "../common.hpp"
#include "../world_init.hpp"

class PickUpSystem
{
public:
  void step(float elapsed_ms);

  PickUpSystem() {}

private:
  void on_pickup(Entity pickup, Entity player);
};
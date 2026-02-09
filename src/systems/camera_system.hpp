#pragma once
#include "../common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"

class CameraSystem {
  public:
    void step();

    CameraSystem() {}

  private:
    const float interp_coefficient = 0.03;
};
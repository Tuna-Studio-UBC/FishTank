#pragma once

#include "state_types.hpp"

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class State {
  protected:
    float timer_ms = 0.f;
    float duration_ms = 0.f;

  public:
    virtual ~State() = default;

    virtual void on_enter(Entity e) {}
    virtual void on_exit(Entity e) {}
    virtual void update(Entity e, float elapsed_ms) = 0;

    template <typename Registry> void request_transition(Registry &registry, Entity e, STATE_TYPE next) {
        registry.stateTransitionRequests.replace(e, next);
    }
};

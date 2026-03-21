#include "state.hpp"

#include "tinyECS/registry.hpp"

class IdleState : public State {
  public:
    void update(Entity e, float elapsed_ms) override {}
};

class PatternAState : public State {
  private:
    float timer_ms = 0.0f;
    float duration_ms = 8 * 1000.f;

  public:
    void on_enter(Entity e) override { timer_ms = 0.0f; }

    void update(Entity e, float elapsed_ms) override {
        timer_ms += elapsed_ms;

        if (timer_ms >= duration_ms) {
            request_transition(registry, e, STATE_TYPE::PATTERN_B);
        }
    }
};

class PatternBState : public State {
  private:
    float timer_ms = 0.0f;
    float duration_ms = 6 * 1000.f;

  public:
    void on_enter(Entity e) override { timer_ms = 0.0f; }

    void update(Entity e, float elapsed_ms) override {
        timer_ms += elapsed_ms;

        if (timer_ms >= duration_ms) {
            request_transition(registry, e, STATE_TYPE::PATTERN_A);
        }
    }
};

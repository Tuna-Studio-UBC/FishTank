#pragma once

#include "state.hpp"

#include "configs/attack_pattern.cpp"
#include "configs/enemy_config.cpp"
#include "tinyECS/registry.hpp"

class BossState : public State {
  protected:
    void switch_attack_pattern(Entity e, int next_pattern) {
        Boss &boss = registry.bosses.get(e);
        boss.curr_attack_pattern_index = next_pattern;

        auto config = BOSS_CONFIGS[(int)boss.type];
        FireControl &fc = registry.fireControls.get(e);
        fc.attack_pattern = config.attack_patterns[next_pattern];
        fc.cooldown_ms = ATTACK_CONFIGS[(int)fc.attack_pattern].cooldown_ms;
    }
};

class IdleState : public BossState {
  public:
    void update(Entity e, float elapsed_ms) override {}
};

#pragma region Sharky
// =================================================================================
// Sharky
// =================================================================================

class SharkyIdleState : public IdleState {};
class SharkyState : public BossState {};

class SharkyNormalState : public SharkyState {
  private:
    float timer_ms = 0.0f;
    float duration_ms = 8 * 1000.f;

  public:
    void on_enter(Entity e) override { timer_ms = 0.0f; }

    void update(Entity e, float elapsed_ms) override {
        timer_ms += elapsed_ms;

        switch_attack_pattern(e, 0);

        if (timer_ms >= duration_ms) {
            request_transition(registry, e, STATE_TYPE::SHARKY_WAVES);
        }
    }
};

class SharkyWavesState : public SharkyState {
  private:
    float timer_ms = 0.0f;
    float duration_ms = 6 * 1000.f;

  public:
    void on_enter(Entity e) override { timer_ms = 0.0f; }

    void update(Entity e, float elapsed_ms) override {
        timer_ms += elapsed_ms;

        switch_attack_pattern(e, 1);

        if (timer_ms >= duration_ms) {
            request_transition(registry, e, STATE_TYPE::SHARKY_NORMAL);
        }
    }
};

class SharkyEnragedState : public SharkyState {
  public:
    void update(Entity e, float elapsed_ms) override {
        Boss &boss = registry.bosses.get(e);
        boss.curr_phase = 2;

        switch_attack_pattern(e, 1);

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = config.move_speed * 1.5f;
        sc.turn_speed = config.turn_speed * 1.5f;

        // Grow projectiles
        for (Entity be : registry.bullets.entities) {
            Bullet &bullet = registry.bullets.get(be);
            if (bullet.owner == e && !registry.growths.has(be)) {
                Growth &growth = registry.growths.emplace(be);
                growth.target_scale = vec2(PROJECTILE_SIZE_PX, PROJECTILE_SIZE_PX) * 2.f;
                growth.rate_ps = 2.f;
            }
        }
    }
};

#pragma endregion Sharky

#pragma region King Crabby
// =================================================================================
// King Crabby
// =================================================================================

class KingCrabbyIdleState : public IdleState {};
class KingCrabbyState : public BossState {};

class KingCrabbyNormalState : public KingCrabbyState {
  public:
    void update(Entity e, float elapsed_ms) override { switch_attack_pattern(e, 0); }
};

class KingCrabbyToxicState : public KingCrabbyState {
  public:
    void update(Entity e, float elapsed_ms) override {
        Boss &boss = registry.bosses.get(e);
        boss.curr_phase = 2;

        switch_attack_pattern(e, 1);

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = config.move_speed * 1.5f;
        sc.turn_speed = config.turn_speed * 1.5f;
    }
};

class KingCrabbyEnragedState : public KingCrabbyState {
  public:
    void update(Entity e, float elapsed_ms) override {
        Boss &boss = registry.bosses.get(e);
        boss.curr_phase = 2;

        switch_attack_pattern(e, 2);

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = 0.f;
        sc.turn_speed = config.turn_speed * 2.f;

        // Adjust attack window
        FireControl &fc = registry.fireControls.get(e);
        fc.can_shoot_ms = 6 * 1000.f;
        fc.cd_can_shoot_ms = 4 * 1000.f;

        // Regenerate health
        Health &health = registry.healths.get(e);
        if (health.hp <= health.max_hp * 0.5f)
            health.hp += 0.05f * elapsed_ms;

        // Grow projectiles
        for (Entity be : registry.bullets.entities) {
            Bullet &bullet = registry.bullets.get(be);
            if (bullet.owner == e && !registry.growths.has(be)) {
                Growth &growth = registry.growths.emplace(be);
                growth.target_scale = vec2(PROJECTILE_SIZE_PX, PROJECTILE_SIZE_PX) * 2.f;
                growth.rate_ps = 10.f;
            }
        }
    }
};

#pragma endregion King Crabby

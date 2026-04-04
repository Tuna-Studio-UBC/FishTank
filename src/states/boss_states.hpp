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

        // Change sprite
        RenderRequest &rr = registry.renderRequests.get(e);
        rr.used_texture = TEXTURE_ASSET_ID::TEXTURE_SHARKY_ENRAGED_SHEET;

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
                growth.target_scale = vec2(PROJECTILE_SIZE_PX, PROJECTILE_SIZE_PX) * 1.75f;
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
        boss.curr_phase = 3;

        switch_attack_pattern(e, 2);

        // Change sprite
        RenderRequest &rr = registry.renderRequests.get(e);
        rr.used_texture = TEXTURE_ASSET_ID::TEXTURE_KING_CRABBY_ENRAGED_SHEET;

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = config.move_speed * 1.5f;
        sc.turn_speed = config.turn_speed * 2.f;

        // Adjust attack window
        FireControl &fc = registry.fireControls.get(e);
        fc.can_shoot_ms = 3 * 1000.f;
        fc.cd_can_shoot_ms = 5 * 1000.f;

        // Regenerate health
        Health &health = registry.healths.get(e);
        if (health.hp <= health.max_hp * 0.5f && !fc.can_shoot)
            health.hp += 10.f * (elapsed_ms / 1000.f);

        // Speed up projectiles
        for (Entity be : registry.bullets.entities) {
            Bullet &bullet = registry.bullets.get(be);
            if (bullet.owner == e) {
                Motion &motion = registry.motions.get(be);
                motion.velocity += 0.25f * motion.velocity * (elapsed_ms / 1000.f);
            }
        }
    }
};

#pragma endregion King Crabby

#pragma region EEEEL
// =================================================================================
// EEEEL
// =================================================================================

class EeeelIdleState : public IdleState {};
class EeeelState : public BossState {};

class EeeelNormalState : public EeeelState {
  public:
    void update(Entity e, float elapsed_ms) override { switch_attack_pattern(e, 0); }
};

// TODO
class EeeelDefensiveSwirlState : public EeeelState {
  public:
    void update(Entity e, float elapsed_ms) override {
        Boss &boss = registry.bosses.get(e);
        boss.curr_phase = 2;

        switch_attack_pattern(e, 1);

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = 0;
        sc.turn_speed = config.turn_speed * 2.f;

        // Adjust attack window
        FireControl &fc = registry.fireControls.get(e);
        fc.can_shoot_ms = 6 * 1000.f;
        fc.cd_can_shoot_ms = 1 * 1000.f;

        // Regenerate health
        Health &health = registry.healths.get(e);
        if (health.hp <= health.max_hp * 0.5f && !fc.can_shoot)
            health.hp += 10.f * (elapsed_ms / 1000.f);

        // Speed up projectiles
        for (Entity be : registry.bullets.entities) {
            Bullet &bullet = registry.bullets.get(be);
            if (bullet.owner == e) {
                Motion &motion = registry.motions.get(be);
                motion.velocity += 0.25f * motion.velocity * (elapsed_ms / 1000.f);
            }
        }
    }
};

class EeeelEnragedState : public EeeelState {
  public:
    void update(Entity e, float elapsed_ms) override {
        Boss &boss = registry.bosses.get(e);
        boss.curr_phase = 3;

        switch_attack_pattern(e, 2);

        // Random chance of spawning isopod (max 10)
        if (registry.enemies.size() < 11) {
            if (get_random_float(0.f, 1.f) < 0.005f) {
                Entity isopod = Entity();
                registry.spawnRequests.insert(isopod, {UNSET_POS, 0.f, ENEMY_TYPE::HOMING});
            }
        }

        // Change sprite
        RenderRequest &rr = registry.renderRequests.get(e);
        rr.used_texture = TEXTURE_ASSET_ID::TEXTURE_EEEEL_ENRAGED_SHEET;

        // Adjust speed
        auto config = BOSS_CONFIGS[(int)boss.type];
        SpeedControl &sc = registry.speedControls.get(e);
        sc.move_speed = config.move_speed * 0.25f;
        sc.turn_speed = config.turn_speed * 2.f;

        // Adjust attack window
        FireControl &fc = registry.fireControls.get(e);
        fc.can_shoot_ms = 3 * 1000.f;
        fc.cd_can_shoot_ms = 5 * 1000.f;

        // Home in projectiles towards player
        for (Entity be : registry.bullets.entities) {
            Bullet &bullet = registry.bullets.get(be);
            if (bullet.owner == e) {
                Motion &motion = registry.motions.get(be);
                Entity player_entity = registry.players.entities[0];
                vec2 player_pos = registry.motions.get(player_entity).position;
                vec2 err = vec2(get_random_float(-10.f, 10.f), get_random_float(-10.f, 10.f));
                vec2 dir = normalize(player_pos + err - motion.position);
                motion.angle = atan2(dir.y, dir.x);
            }
        }
    }
};

#pragma endregion EEEEL

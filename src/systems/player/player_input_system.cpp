// internal
#include "player_input_system.hpp"
#include "systems/world/world_init.hpp"
#include <iostream>

float MAX_SPEED_MULTIPLIER = 2;             // Max speed = move speed * x
float DECELERATION_MULTIPLIER = 4;          // Deceleration = move speed * x
float QUICK_TURN_MULTIPLIER = 2;            // QT speed = turn speed * x
float ANGULAR_VELOCITY_LOSS_MULTIPLIER = 4; // How fast we lose rotation velocity

void debug_print_input_state(KeyboardInputState &keyboard_input) {
    std::cout << "Current:  ";
    for (int i = 0; i < KeyboardActionsCount; ++i) {
        std::cout << keyboard_input.current_actions[i] << " ";
    }
    std::cout << "\nPrevious: ";
    for (int i = 0; i < KeyboardActionsCount; ++i) {
        std::cout << keyboard_input.previous_actions[i] << " ";
    }
    std::cout << std::endl;
}

void debug_print_player_motion(Motion &motion) {
    std::cout << "(" << motion.position.x << "," << motion.position.y << ") <" << motion.velocity.x << ","
              << motion.velocity.y << "> " << motion.angle << std::endl;
}

bool key_pressed(KeyboardActions a, const KeyboardInputState &keyboard_input) {
    return keyboard_input.current_actions[a] && !keyboard_input.previous_actions[a];
}

bool key_held(KeyboardActions a, const KeyboardInputState &keyboard_input) { return keyboard_input.current_actions[a]; }

// TODO: STUB FUNCTION -- REPLACE WHEN WE IMPLEMENT DIFFERENT AMMO TYPES
int PlayerInputSystem::get_player_ammo_cost() { return 1; }


   // Tries to fire left click projectile (normal bullet for Gun 1, explosive for Gun 2)

void PlayerInputSystem::try_fire_left_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    // Gun 2 explosive bullets cost 10 ammo, Gun 1 costs 1
    int ammo_cost = (ws.current_weapon == WeaponType::GUN_2) ? 10 : 1;
    if (fc.ammo < ammo_cost)
        return;

    // Check cooldown based on weapon type
    float cooldown = (ws.current_weapon == WeaponType::GUN_1) ? ws.gun1_left_cooldown : ws.gun2_left_cooldown;
    if (game_time_ms - ws.last_left_click_time < cooldown)
        return;

    float barrel_len = DEFAULT_BARREL_LEN;
    if (!registry.turrets.entities.empty()) {
        Entity turret_entity = registry.turrets.entities[0];
        barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
    }

    vec2 forward = {cosf(aim_angle), sinf(aim_angle)};
    vec2 bullet_spawn_pos = player_motion.position + forward * barrel_len;
    Entity player = registry.players.entities[0];

    if (ws.current_weapon == WeaponType::GUN_1) {
        // Gun 1: Normal bouncing bullet (1 ammo)
        createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);
    } else {
        // Gun 2: Explosive bullet (10 ammo, no bouncing)
        Entity bullet = createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);
        registry.bouncies.remove(bullet);  // Remove bouncy component
        Bullet &b = registry.bullets.get(bullet);
        b.type = BulletType::EXPLOSIVE;
        registry.explosives.emplace(bullet, Explosive{125.0f, 20.0f, false});

        // Make it a red bubble
        RenderRequest &rr = registry.renderRequests.get(bullet);
        rr.used_texture = TEXTURE_ASSET_ID::TEXTURE_BUBBLE;

        // Add red tint
        if (registry.tints.has(bullet)) {
            registry.tints.get(bullet).tint = vec3(1.0f, 0.1f, 0.1f);  // Red
        } else {
            registry.tints.emplace(bullet, vec3(1.0f, 0.1f, 0.1f));
        }
    }

    auto &sr = registry.sound_requests.emplace_with_duplicates(player);
    sr.type = SOUND_EVENT::PLAYER_SHOT;

    ws.last_left_click_time = game_time_ms;
    fc.ammo -= ammo_cost;
    Scoring &score = registry.scoring.components[0];
    score.bullets_fired += 1;
}


  // Tries to fire right click projectile (burst for Gun 1, mine for Gun 2)

void PlayerInputSystem::try_fire_right_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    // Gun 2 mine costs 10 ammo, Gun 1 burst costs 1 per shot
    int ammo_cost = (ws.current_weapon == WeaponType::GUN_2) ? 5 : 1;
    if (fc.ammo < ammo_cost)
        return;

    float barrel_len = DEFAULT_BARREL_LEN;
    if (!registry.turrets.entities.empty()) {
        Entity turret_entity = registry.turrets.entities[0];
        barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
    }

    vec2 forward = {cosf(aim_angle), sinf(aim_angle)};
    vec2 bullet_spawn_pos = player_motion.position + forward * barrel_len;
    Entity player = registry.players.entities[0];

    if (ws.current_weapon == WeaponType::GUN_1) {
        // Gun 1: Burst fire (3 shots)
        if (game_time_ms - ws.last_right_click_time < ws.gun1_right_cooldown)
            return;

        ws.burst_shots_remaining = 3;
        ws.next_burst_time = game_time_ms;
        ws.last_right_click_time = game_time_ms;
    } else {
        // Gun 2: Deploy mine
        if (game_time_ms - ws.last_right_click_time < ws.gun2_right_cooldown)
            return;

        Entity mine = createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);
        registry.bouncies.remove(mine);  // don't bounce

        Bullet &b = registry.bullets.get(mine);
        b.type = BulletType::DEPLOYED_MINE;

        Motion &mine_motion = registry.motions.get(mine);
        mine_motion.velocity = vec2(150.0f, 0.f);  // slower speed
        mine_motion.scale = vec2(40.f, 40.f);      // match hazard mine size

        // Use mine sprite sheet with animation (green tint instead of red)
        RenderRequest &rr = registry.renderRequests.get(mine);
        rr.used_texture = TEXTURE_ASSET_ID::TEXTURE_MINE_SHEET;
        rr.layer = RENDER_LAYER::INTERACTABLES;

        // Add animation (same as hazard mine: 3 frames flashing)
        Animation &an = registry.animations.emplace(mine);
        an.columns = 3;
        an.frame_count = 3;
        an.frame_time = 500;

        // Green tint to distinguish from red hazard mines
        if (registry.tints.has(mine)) {
            registry.tints.get(mine).tint = vec3(0.4f, 1.0f, 0.4f);  // Green
        } else {
            registry.tints.emplace(mine, vec3(0.4f, 1.0f, 0.4f));
        }

        registry.explosives.emplace(mine, Explosive{60.0f, 30.0f, false}); //radius , damage, can chain react
        registry.deployedMines.emplace(mine, DeployedMine{75.0f, false});
        registry.expirations.get(mine).lifespan_ms = 40000.0f;  // 40 seconds

        auto &sr = registry.sound_requests.emplace_with_duplicates(player);
        sr.type = SOUND_EVENT::PLAYER_SHOT;

        ws.last_right_click_time = game_time_ms;
        fc.ammo -= ammo_cost;
    }
}


   // Processes burst fire for Gun 1

void PlayerInputSystem::process_burst_fire(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    if (ws.burst_shots_remaining > 0 && game_time_ms >= ws.next_burst_time) {
        if (fc.ammo < 1) {
            ws.burst_shots_remaining = 0;
            return;
        }

        float barrel_len = DEFAULT_BARREL_LEN;
        if (!registry.turrets.entities.empty()) {
            Entity turret_entity = registry.turrets.entities[0];
            barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
        }

        vec2 forward = {cosf(aim_angle), sinf(aim_angle)};
        vec2 bullet_spawn_pos = player_motion.position + forward * barrel_len;
        Entity player = registry.players.entities[0];

        createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);

        auto &sr = registry.sound_requests.emplace_with_duplicates(player);
        sr.type = SOUND_EVENT::PLAYER_SHOT;

        fc.ammo -= 1;
        ws.burst_shots_remaining--;
        ws.next_burst_time = game_time_ms + 100.0f;  // 100ms between burst shots
    }
}

void PlayerInputSystem::step(float elapsed_ms) {
    // We should have 1 and only ever 1 player input component
    assert(registry.keyboardInputs.components.size() == 1);
    assert(registry.mouseInputs.components.size() == 1);
    Entity playerEntity = registry.players.entities[0];
    KeyboardInputState &keyboard_input = registry.keyboardInputs.get(playerEntity);
    MouseInputState &mouse_input = registry.mouseInputs.get(playerEntity);
    Motion &motion = registry.motions.get(playerEntity);
    SpeedControl &sc = registry.speedControls.get(playerEntity);

    // Update game time
    game_time_ms += elapsed_ms;

    // Get weapon state
    WeaponState &ws = registry.weaponStates.get(playerEntity);

    float dt = elapsed_ms / 1000.0f;
    bool quick_turn_active = (registry.playerQuickTurns.size() != 0);
    float quick_turn_duration = (M_PI / sc.turn_speed) * 1000.f / QUICK_TURN_MULTIPLIER;

    // Process Turn Input
    if (key_pressed(KeyboardActions::QuickTurn, keyboard_input) && !quick_turn_active) {
        Entity e = Entity("PlayerQuickTurn");
        PlayerQuickTurn &qt = registry.playerQuickTurns.emplace(e);
        motion.angular_velocity = 0;
        if (keyboard_input.most_recent_turn == KeyboardActions::TurnLeft) {
            qt.direction = -1;
        } else {
            qt.direction = 1;
        }
        qt.duration_ms = quick_turn_duration;
    } else if (!quick_turn_active) {
        float turn_dir = 0.f;
        if (key_held(KeyboardActions::TurnLeft, keyboard_input)) {
            turn_dir -= 1.f;
        }
        if (key_held(KeyboardActions::TurnRight, keyboard_input)) {
            turn_dir += 1.f;
        }
        // motion.angle = std::fmod(motion.angle + turn_dir * sc.turn_speed * dt, 2 * M_PI);
        if (turn_dir != 0.f) {
            motion.angular_velocity = turn_dir * sc.turn_speed;
        } else if (motion.angular_velocity > 0) {
            motion.angular_velocity =
                std::max(0.f, motion.angular_velocity - sc.turn_speed * dt * ANGULAR_VELOCITY_LOSS_MULTIPLIER);
        } else if (motion.angular_velocity < 0) {
            motion.angular_velocity =
                std::min(0.f, motion.angular_velocity + sc.turn_speed * dt * ANGULAR_VELOCITY_LOSS_MULTIPLIER);
        }
    }

    // Process Quick Turn
    if (quick_turn_active) {
        assert(registry.playerQuickTurns.size() == 1);
        PlayerQuickTurn &qt = registry.playerQuickTurns.components[0];
        qt.duration_ms -= elapsed_ms;

        float relevant_ms = elapsed_ms;
        if (qt.duration_ms < 0) {
            relevant_ms += qt.duration_ms;
        }

        float quick_turn_amount = (M_PI * relevant_ms * qt.direction) / quick_turn_duration;
        motion.angle = std::fmod(motion.angle + quick_turn_amount, 2 * M_PI);

        if (qt.duration_ms <= 0) {
            for (Entity &e : registry.playerQuickTurns.entities) {
                registry.remove_all_components_of(e);
            }
        }
    }

    if (key_held(KeyboardActions::MoveForward, keyboard_input)) {
        float max_speed = 3.f * sc.move_speed;
        motion.velocity.x = std::min(max_speed, motion.velocity.x + sc.move_speed * dt);
    } else {
        motion.velocity.x = std::max(0.f, motion.velocity.x - (DECELERATION_MULTIPLIER * sc.move_speed) * dt);
    }

    FireControl &fc = registry.fireControls.get(playerEntity);
    fc.cooldown_timer = std::max(0.f, fc.cooldown_timer - elapsed_ms);

    // ===== Turret aim =====
    vec2 player_pos = motion.position;
    vec2 &camera_pos = registry.motions.get(registry.cameras.entities[0]).position;
    vec2 mouse_relative_pos =
        mouse_input.mouse_pos + camera_pos - player_pos - vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
    float aim_angle = atan2(mouse_relative_pos.y, mouse_relative_pos.x);

    if (!registry.turrets.entities.empty()) {
        Entity turret_entity = registry.turrets.entities[0];
        Motion &tm = registry.motions.get(turret_entity);
        tm.position = player_pos;
        tm.angle = aim_angle;
    }

    // Process burst fire
    process_burst_fire(motion, fc, ws, aim_angle);

    if (mouse_input.current_action == MouseActions::LeftClick) {
        try_fire_left_click(motion, fc, ws, aim_angle);
    }
    if (mouse_input.current_action == MouseActions::RightClick) {
        try_fire_right_click(motion, fc, ws, aim_angle);
    }

    // Shift action over
    std::copy(std::begin(keyboard_input.current_actions), std::end(keyboard_input.current_actions),
              std::begin(keyboard_input.previous_actions));
}

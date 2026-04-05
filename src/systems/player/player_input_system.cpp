// internal
#include "player_input_system.hpp"
#include "systems/world/world_init.hpp"
#include <iostream>

float MAX_SPEED_MULTIPLIER = 2.8;             // Max speed = move speed * x
float MAX_REVERSE_MULTIPLIER = 0.5;             // Max speed = move speed * x
float ACCELERATION_MULTIPLIER = 3;
float DECELERATION_MULTIPLIER = 5;          // Deceleration = move speed * x
float AUTO_ACCELERATION_MULTIPLIER = 2;          // Deceleration = move speed * x
float AUTO_DECELERATION_MULTIPLIER = 2;          // Deceleration = move speed * x
float ANGULAR_VELOCITY_MULTIPLIER = 0.8;
float ANGULAR_VELOCITY_LOSS_MULTIPLIER = 4; // How fast we lose rotation velocity

// Alternative WASD controls are tuned differently
float ALT_AUTO_DECELERATION_MULTIPLIER = 4;
float ALT_ANGULAR_VELOCITY_MULTIPLIER = 1.0;
float ALT_ACCELERATION_MULTIPLIER = 5;

bool USE_ALTERNATIVE_INPUT = false;

bool* PlayerInputSystem::get_input_mode() {
    return &USE_ALTERNATIVE_INPUT;
}

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

namespace {
void queue_no_ammo_sound(Entity player, WeaponState &ws, float game_time_ms) {
    if (game_time_ms - ws.last_no_ammo_sound_time < ws.no_ammo_sound_cooldown)
        return;

    auto &sr = registry.sound_requests.emplace_with_duplicates(player);
    sr.type = SOUND_EVENT::NO_AMMO;
    ws.last_no_ammo_sound_time = game_time_ms;
}
} // namespace


   // Tries to fire left click projectile (normal bullet for Gun 1, explosive for Gun 2)

void PlayerInputSystem::try_fire_left_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    // Gun 2 explosive bullets cost 10 ammo, Gun 3 costs 0.5 (every 2 shots), Gun 1 costs 1
    int ammo_cost = (ws.current_weapon == WeaponType::GUN_2) ? 10 : 1;
    Entity player = registry.players.entities[0];

    // For gun 3, check if we can fire (need ammo or have shot credit)
    if (ws.current_weapon == WeaponType::GUN_3) {
        if (fc.ammo <= 0 && ws.gun3_shots_since_ammo_consumed == 0) {
            queue_no_ammo_sound(player, ws, game_time_ms);
            return;
        }
    } else {
        if (fc.ammo < ammo_cost) {
            queue_no_ammo_sound(player, ws, game_time_ms);
            return;
        }
    }

    // Check cooldown based on weapon type
    float cooldown = 0.0f;
    if (ws.current_weapon == WeaponType::GUN_1) {
        cooldown = ws.gun1_left_cooldown;
    } else if (ws.current_weapon == WeaponType::GUN_2) {
        cooldown = ws.gun2_left_cooldown;
    } else if (ws.current_weapon == WeaponType::GUN_3) {
        cooldown = ws.gun3_left_cooldown;
    }

    if (game_time_ms - ws.last_left_click_time < cooldown)
        return;

    float barrel_len = DEFAULT_BARREL_LEN;
    if (!registry.turrets.entities.empty()) {
        Entity turret_entity = registry.turrets.entities[0];
        barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
    }

    vec2 forward = {cosf(aim_angle), sinf(aim_angle)};
    vec2 bullet_spawn_pos = player_motion.position + forward * barrel_len;

    if (ws.current_weapon == WeaponType::GUN_1) {
        // Gun 1: Normal bouncing bullet (1 ammo)
        createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);
    } else if (ws.current_weapon == WeaponType::GUN_2) {
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
    } else if (ws.current_weapon == WeaponType::GUN_3) {
        // Gun 3: Minigun - low damage rapid fire (1 ammo)
        Entity bullet = createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, 0);  // No damage bonus

        // Override bullet damage: base 2 + small fraction of damage bonus
        if (registry.bullets.has(bullet)) {
            Bullet &b = registry.bullets.get(bullet);
            b.damage = 3 + (fc.damage_bonus / 5);  // Base 3 damage + 20% of damage bonus
        }

        // Make bullet smaller and faster for minigun feel
        if (registry.motions.has(bullet)) {
            Motion &bullet_motion = registry.motions.get(bullet);
            bullet_motion.scale *= 0.7f;  // Smaller bullets
            bullet_motion.velocity *= 1.3f;  // Faster bullets
        }

        // Green tint for minigun bullets
        if (registry.tints.has(bullet)) {
            registry.tints.get(bullet).tint = vec3(0.2f, 1.0f, 0.2f);  // Green
        } else {
            registry.tints.emplace(bullet, vec3(0.2f, 1.0f, 0.2f));
        }
    }

    auto &sr = registry.sound_requests.emplace_with_duplicates(player);
    sr.type = (ws.current_weapon == WeaponType::GUN_2) ? SOUND_EVENT::PLAYER_CANNON_SHOT : SOUND_EVENT::PLAYER_SHOT;

    ws.last_left_click_time = game_time_ms;

    // Handle ammo consumption for gun 3 (half rate)
    if (ws.current_weapon == WeaponType::GUN_3) {
        ws.gun3_shots_since_ammo_consumed++;
        if (ws.gun3_shots_since_ammo_consumed >= 2) {
            fc.ammo -= 1;
            ws.gun3_shots_since_ammo_consumed = 0;
        }
    } else {
        fc.ammo -= ammo_cost;
    }

    Scoring &score = registry.scoring.components[0];
    score.bullets_fired += 1;
}


  // Tries to fire right click projectile (burst for Gun 1, mine for Gun 2)

void PlayerInputSystem::try_fire_right_click(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    // Gun 2 mine costs 5 ammo, Gun 1 burst costs 1 per shot, Gun 3 full-auto uses half rate
    int ammo_cost = 1;
    if (ws.current_weapon == WeaponType::GUN_2) {
        ammo_cost = 5;
    }

    Entity player = registry.players.entities[0];

    // For gun 3 right-click, check if we can start full-auto (need ammo or have shot credit)
    if (ws.current_weapon == WeaponType::GUN_3) {
        if (fc.ammo <= 0 && ws.gun3_shots_since_ammo_consumed == 0) {
            queue_no_ammo_sound(player, ws, game_time_ms);
            return;
        }
    } else {
        if (fc.ammo < ammo_cost) {
            queue_no_ammo_sound(player, ws, game_time_ms);
            return;
        }
    }

    float barrel_len = DEFAULT_BARREL_LEN;
    if (!registry.turrets.entities.empty()) {
        Entity turret_entity = registry.turrets.entities[0];
        barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
    }

    vec2 forward = {cosf(aim_angle), sinf(aim_angle)};
    vec2 bullet_spawn_pos = player_motion.position + forward * barrel_len;

    if (ws.current_weapon == WeaponType::GUN_1) {
        // Gun 1: Burst fire (3 shots)
        if (game_time_ms - ws.last_right_click_time < ws.gun1_right_cooldown)
            return;

        ws.burst_shots_remaining = 3;
        ws.next_burst_time = game_time_ms;
        ws.last_right_click_time = game_time_ms;
    } else if (ws.current_weapon == WeaponType::GUN_2) {
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
        sr.type = SOUND_EVENT::PLAYER_SHOT_MINE;

        ws.last_right_click_time = game_time_ms;
        fc.ammo -= ammo_cost;
    } else if (ws.current_weapon == WeaponType::GUN_3) {
        // Gun 3: mini-gun mode / dump all remaining ammo rapidly (at half rate)
        if (game_time_ms - ws.last_right_click_time < ws.gun3_right_cooldown)
            return;

        // Calculate total shots available: current ammo * 2 + any extra shot credit
        ws.minigun_shots_remaining = (fc.ammo * 2) + ws.gun3_shots_since_ammo_consumed;
        ws.next_minigun_time = game_time_ms;
        ws.last_right_click_time = game_time_ms;
    }
}


   // Processes burst fire for Gun 1

void PlayerInputSystem::process_burst_fire(Motion &player_motion, FireControl &fc, WeaponState &ws, const float &aim_angle) {
    if (ws.burst_shots_remaining > 0 && game_time_ms >= ws.next_burst_time) {
        Entity player = registry.players.entities[0];
        if (fc.ammo < 1) {
            queue_no_ammo_sound(player, ws, game_time_ms);
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

        createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, fc.damage_bonus);

        auto &sr = registry.sound_requests.emplace_with_duplicates(player);
        sr.type = SOUND_EVENT::PLAYER_SHOT;

        fc.ammo -= 1;
        ws.burst_shots_remaining--;
        ws.next_burst_time = game_time_ms + 100.0f;  // 100ms between burst shots
    }

    // minigun full-auto fire (GUN_3)
    if (ws.minigun_shots_remaining > 0 && game_time_ms >= ws.next_minigun_time) {
        // Check if we can continue shooting (need ammo or have shot credit)
        if (fc.ammo <= 0 && ws.gun3_shots_since_ammo_consumed == 0) {
            ws.minigun_shots_remaining = 0;
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

        // Create minigun bullet with low damage
        Entity bullet = createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player, 0);  // No damage bonus

        // Override bullet damage: base 2 + small fraction of damage bonus
        if (registry.bullets.has(bullet)) {
            Bullet &b = registry.bullets.get(bullet);
            b.damage = 2 + (fc.damage_bonus / 5);  // Base 2 damage + 20% of damage bonus
        }

        // Make bullet smaller and faster
        if (registry.motions.has(bullet)) {
            Motion &bullet_motion = registry.motions.get(bullet);
            bullet_motion.scale *= 0.7f;
            bullet_motion.velocity *= 1.3f;
        }

        // Green tint
        if (registry.tints.has(bullet)) {
            registry.tints.get(bullet).tint = vec3(0.2f, 1.0f, 0.2f);
        } else {
            registry.tints.emplace(bullet, vec3(0.2f, 1.0f, 0.2f));
        }

        auto &sr = registry.sound_requests.emplace_with_duplicates(player);
        sr.type = SOUND_EVENT::PLAYER_SHOT;

        // Handle ammo consumption for minigun (half rate)
        ws.gun3_shots_since_ammo_consumed++;
        if (ws.gun3_shots_since_ammo_consumed >= 2) {
            fc.ammo -= 1;
            ws.gun3_shots_since_ammo_consumed = 0;
        }

        ws.minigun_shots_remaining--;
        ws.next_minigun_time = game_time_ms + 50.0f;  // 50ms between minigun shots (twice as fast as burst)

        Scoring &score = registry.scoring.components[0];
        score.bullets_fired += 1;
    }
}

void handle_tank_controls(float elapsed_ms, KeyboardInputState &keyboard_input, Motion &motion, SpeedControl &sc) {
    float dt = elapsed_ms / 1000.0f;

    float turn_dir = 0.f;
    if (key_held(KeyboardActions::TurnLeft, keyboard_input)) {
        turn_dir -= 1.f;
    }
    if (key_held(KeyboardActions::TurnRight, keyboard_input)) {
        turn_dir += 1.f;
    }

    // motion.angle = std::fmod(motion.angle + turn_dir * sc.turn_speed * dt, 2 * M_PI);
    if (turn_dir != 0.f) {
        motion.angular_velocity = turn_dir * sc.turn_speed * ANGULAR_VELOCITY_MULTIPLIER;
    } else if (motion.angular_velocity > 0) {
        motion.angular_velocity =
            std::max(0.f, motion.angular_velocity - sc.turn_speed * dt * ANGULAR_VELOCITY_LOSS_MULTIPLIER);
    } else if (motion.angular_velocity < 0) {
        motion.angular_velocity =
            std::min(0.f, motion.angular_velocity + sc.turn_speed * dt * ANGULAR_VELOCITY_LOSS_MULTIPLIER);
    }

    float max_speed = MAX_SPEED_MULTIPLIER * sc.move_speed;
    float max_reverse = MAX_REVERSE_MULTIPLIER * sc.move_speed;

    if (key_held(KeyboardActions::MoveForward, keyboard_input)) {
        motion.velocity.x = std::min(max_speed, motion.velocity.x + (sc.move_speed * dt * ACCELERATION_MULTIPLIER));
    } else if (key_held(KeyboardActions::MoveBackward, keyboard_input)) {
        motion.velocity.x = std::max(-max_reverse, motion.velocity.x - (DECELERATION_MULTIPLIER * sc.move_speed * dt));
    } else {
        if (motion.velocity.x > 0) {
            motion.velocity.x = std::max(0.f, motion.velocity.x - (AUTO_DECELERATION_MULTIPLIER * sc.move_speed) * dt);
        } else if (motion.velocity.x < 0) {
            motion.velocity.x = std::min(0.f, motion.velocity.x + (AUTO_ACCELERATION_MULTIPLIER * sc.move_speed) * dt);
        }
    }
}

float get_desired_heading(KeyboardInputState &keyboard_input, float current_angle) {
    auto &act = keyboard_input.current_actions;
    float dir_x = 0;
    float dir_y = 0;

    if (act[KeyboardActions::MoveForward]) {
        dir_y -= 1;
    }
    if (act[KeyboardActions::MoveBackward]) {
        dir_y += 1;
    }
    if (act[KeyboardActions::TurnLeft]) {
        dir_x -= 1;
    }
    if (act[KeyboardActions::TurnRight]) {
        dir_x += 1;
    }

    // Check final result
    if (dir_x == 0.f && dir_y == 0.f) {
        return current_angle;
    }

    return std::atan2(dir_y, dir_x);
}

bool valid_movement(KeyboardInputState &keyboard_input) {
    auto &act = keyboard_input.current_actions;
    float dir_x = 0;
    float dir_y = 0;

    if (act[KeyboardActions::MoveForward]) {
        dir_y -= 1;
    }
    if (act[KeyboardActions::MoveBackward]) {
        dir_y += 1;
    }
    if (act[KeyboardActions::TurnLeft]) {
        dir_x -= 1;
    }
    if (act[KeyboardActions::TurnRight]) {
        dir_x += 1;
    }

    return !(dir_x == 0.f && dir_y == 0.f);
}

void handle_wasd_controls(float elapsed_ms, KeyboardInputState &keyboard_input, Motion &motion, SpeedControl &sc) {
    float dt = elapsed_ms / 1000.0f;

    float desired_heading = get_desired_heading(keyboard_input, motion.angle);
    
    float delta = desired_heading - motion.angle;
    while (delta > M_PI) {
        delta -= 2 * M_PI;
    }
    while (delta < -M_PI) {
        delta += 2 * M_PI;
    }
    
    float max_turn = sc.turn_speed * dt * ALT_ANGULAR_VELOCITY_MULTIPLIER * 1.5;
    if (delta > max_turn) {
        delta = max_turn;
    } else if (delta < -max_turn) {
        delta = -max_turn;
    }

    motion.angle += delta;
    float max_speed = MAX_SPEED_MULTIPLIER * sc.move_speed;

    // Accept any valid movement input for acceleration
    if (valid_movement(keyboard_input)) {
        motion.velocity.x = std::min(max_speed, motion.velocity.x + (sc.move_speed * dt * 2 * ALT_ACCELERATION_MULTIPLIER));
    } else {
        motion.velocity.x = std::max(0.f, motion.velocity.x - (ALT_AUTO_DECELERATION_MULTIPLIER * 2 * sc.move_speed) * dt);
    }

    // spdlog::debug("{}", motion.angle);
    // spdlog::debug("{}", desired_heading);
    // spdlog::debug("{}", delta);
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

    if (USE_ALTERNATIVE_INPUT) {
        handle_wasd_controls(elapsed_ms, keyboard_input, motion, sc);
    } else {
        handle_tank_controls(elapsed_ms, keyboard_input, motion, sc);
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

    // Parry
    ParryState &parry = registry.parryStates.get(playerEntity);

    // Update cooldown timer
    if (parry.cooldown_timer > 0.0f) {
        parry.cooldown_timer = std::max(0.0f, parry.cooldown_timer - elapsed_ms);
    }

    // Update parry window timer (iframes)
    if (parry.is_active) {
        parry.window_timer -= elapsed_ms;
        if (parry.window_timer <= 0.0f) {
            parry.is_active = false;
            parry.window_timer = 0.0f;
        }
    }

    // Dash
    DashState &dash = registry.dashStates.get(playerEntity);

    // Update cooldown timer
    if (dash.cooldown_timer > 0.0f) {
        dash.cooldown_timer = std::max(0.0f, dash.cooldown_timer - elapsed_ms);
    }

    // Update dash duration timer
    if (dash.is_dashing) {
        dash.duration_timer -= elapsed_ms;
        if (dash.duration_timer <= 0.0f) {
            dash.is_dashing = false;
            dash.duration_timer = 0.0f;
        }
    }

    // activate dash on key press (only if not on cooldown, not already dashing, and moving forward)
    if (key_pressed(KeyboardActions::Dash, keyboard_input) && dash.cooldown_timer <= 0.0f && !dash.is_dashing && motion.velocity.x > 250.0f) {
        dash.is_dashing = true;
        dash.duration_timer = dash.duration_ms;
        dash.cooldown_timer = dash.cooldown_ms;
        dash.has_dealt_damage = false;  // Reset damage flag for new dash
    }

    // apply dash velocity while dashing
    if (dash.is_dashing) {
        // Set local forward velocity (the physics system will rotate this based on player angle)
        motion.velocity.x = dash.dash_speed;
        motion.velocity.y = 0.0f;  // No lateral movement during dash
    }

    // Apply tint for parry state (only when parry is active or on cooldown)
    if (parry.is_active || parry.cooldown_timer > 0.0f) {
        vec3 target_tint = vec3(1.0f, 1.0f, 1.0f); // Default

        if (parry.is_active) {
            // Full white during active parry window
            target_tint = vec3(2.0f, 2.0f, 2.0f); // bright white (values > 1 for intensity)
        } else if (parry.cooldown_timer > 0.0f) {
            // Fade from white back to normal as cooldown decreases
            float cooldown_progress = parry.cooldown_timer / parry.cooldown_ms;
            float fade = 1.0f + cooldown_progress * 0.5f; // Fade from 1.5 to 1.0
            target_tint = vec3(fade, fade, fade);
        }

        // Apply tint to player
        if (registry.tints.has(playerEntity)) {
            Tint &player_tint = registry.tints.get(playerEntity);
            // Only override if not a temporary damage tint (life_ms <= 0 means permanent/controlled tint)
            if (player_tint.life_ms <= 0.0f) {
                player_tint.tint = target_tint;
                player_tint.life_ms = -99.f; // Keep it as controlled tint
            }
        } else {
            Tint &new_tint = registry.tints.emplace(playerEntity, target_tint);
            new_tint.life_ms = -99.f; // Controlled tint, not temporary
        }

        // Apply tint to turret (keep weapon specific tint when not in parry)
        if (!registry.turrets.entities.empty()) {
            Entity turret_entity = registry.turrets.entities[0];

            // Only override turret tint during active parry window
            if (parry.is_active) {
                if (registry.tints.has(turret_entity)) {
                    registry.tints.get(turret_entity).tint = target_tint;
                } else {
                    registry.tints.emplace(turret_entity, target_tint);
                }
            } else if (parry.cooldown_timer > 0.0f) {
                // During cooldown fade, blend with the weapon tint
                vec3 weapon_base_tint = vec3(1.0f, 1.0f, 1.0f);
                if (ws.current_weapon == WeaponType::GUN_2) {
                    weapon_base_tint = vec3(1.0f, 0.1f, 0.1f); // Red for GUN_2
                } else if (ws.current_weapon == WeaponType::GUN_3) {
                    weapon_base_tint = vec3(0.2f, 1.0f, 0.2f); // Green for GUN_3
                }

                // Blend between parry fade and weapon color
                float cooldown_progress = parry.cooldown_timer / parry.cooldown_ms;
                float fade_amount = cooldown_progress * 0.5f; // 0 to 0.5
                vec3 blended_tint = weapon_base_tint + vec3(fade_amount, fade_amount, fade_amount);

                if (registry.tints.has(turret_entity)) {
                    registry.tints.get(turret_entity).tint = blended_tint;
                } else {
                    registry.tints.emplace(turret_entity, blended_tint);
                }
            }
        }
    }

    // activate parry on key press (only if not on cooldown)
    if (key_pressed(KeyboardActions::Parry, keyboard_input) && parry.cooldown_timer <= 0.0f && !parry.is_active) {
        parry.is_active = true;
        parry.cooldown_timer = parry.cooldown_ms;

        // Deflect nearby enemy bullets
        vec2 mouse_direction = normalize(mouse_relative_pos);
        float deflect_angle = atan2(mouse_direction.y, mouse_direction.x);

        for (Entity bullet_entity : registry.bullets.entities) {
            if (!registry.motions.has(bullet_entity)) continue;

            Bullet &bullet = registry.bullets.get(bullet_entity);
            Motion &bullet_motion = registry.motions.get(bullet_entity);

            // Only deflect enemy bullets
            if (bullet.owner == playerEntity) continue;

            // Check if bullet is within deflect radius
            vec2 bullet_to_player = bullet_motion.position - player_pos;
            float distance = length(bullet_to_player);

            if (distance <= parry.deflect_radius) {
                // Change ownership to player
                bullet.owner = playerEntity;
                bullet.attack_effect = ATTACK_EFFECT::NORMAL;

                // Apply player damage bonus to the parried bullet
                bullet.damage += fc.damage_bonus;

                // Redirect bullet toward mouse direction
                // Bullets use local velocity that gets rotated by angle in physics system
                float bullet_speed = length(bullet_motion.velocity);
                bullet_motion.angle = deflect_angle;
                bullet_motion.velocity = vec2(bullet_speed, 0.f);  // Local velocity (forward direction)

                // Add visual feedback with a tint
                if (registry.tints.has(bullet_entity)) {
                    registry.tints.get(bullet_entity).tint = vec3(0.3f, 0.3f, 1.0f);  // Blue tint for parried bullets
                } else {
                    registry.tints.emplace(bullet_entity, vec3(0.3f, 0.3f, 1.0f));
                }
            }
        }
    }

    // Shift action over
    std::copy(std::begin(keyboard_input.current_actions), std::end(keyboard_input.current_actions),
              std::begin(keyboard_input.previous_actions));
}

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

/**
    Tries to fire a projectile if the firing cooldown is 0, and the player has sufficient ammo. Does nothing otherwise.
*/
void PlayerInputSystem::try_fire_projectile(Motion &player_motion, FireControl &fc, const float &aim_angle) {
    int ammo_cost = get_player_ammo_cost();
    if (fc.cooldown_timer > 0.f || fc.ammo < ammo_cost)
        return;

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

    fc.cooldown_timer = fc.cooldown_ms;
    fc.ammo -= ammo_cost;
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

    if (mouse_input.current_action == MouseActions::LeftClick) {
        try_fire_projectile(motion, fc, aim_angle);
    }

    // Shift action over
    std::copy(std::begin(keyboard_input.current_actions), std::end(keyboard_input.current_actions),
              std::begin(keyboard_input.previous_actions));
}

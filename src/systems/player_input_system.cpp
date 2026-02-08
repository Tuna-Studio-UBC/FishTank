// internal
#include "player_input_system.hpp"
#include "world_init.hpp"
#include <iostream>

float MAX_SPEED_MULTIPLIER = 2;     // Max speed = move speed * x
float DECELERATION_MULTIPLIER = 2;  // Deceleration = move speed * x

void debug_print_input_state(KeyboardInputState &keyboard_input) {
    std::cout << "Current:  ";
    for (int i = 0; i < KeyboardActionsCount; ++i)
    {
        std::cout << keyboard_input.current_actions[i] << " ";
    }
    std::cout << "\nPrevious: ";
    for (int i = 0; i < KeyboardActionsCount; ++i)
    {
        std::cout << keyboard_input.previous_actions[i] << " ";
    }
    std::cout << std::endl;
}

void debug_print_player_motion(Motion &motion) {
    std::cout << "(" << motion.position.x << "," << motion.position.y << ") <" << motion.velocity.x << "," << motion.velocity.y << "> " << motion.angle << std::endl;
}

bool key_pressed(KeyboardActions a, const KeyboardInputState &keyboard_input) {
    return keyboard_input.current_actions[a] && !keyboard_input.previous_actions[a];
}

bool key_held(KeyboardActions a, const KeyboardInputState &keyboard_input) {
    return keyboard_input.current_actions[a];
}

void PlayerInputSystem::step(float elapsed_ms)
{
    // We should have 1 and only ever 1 player input component
    assert(registry.keyboardInput.components.size() == 1);
    assert(registry.mouseInput.components.size() == 1);
    Entity playerEntity = registry.players.entities[0];
    KeyboardInputState &keyboard_input = registry.keyboardInput.get(playerEntity);
    MouseInputState &mouse_input = registry.mouseInput.get(playerEntity);
    Motion &motion = registry.motions.get(playerEntity);
    SpeedControl &sc = registry.speedControl.get(playerEntity);

    float dt = elapsed_ms / 1000.0f;
    bool quick_turn_active = (registry.playerQuickTurn.size() != 0);
    float quick_turn_duration = (M_PI / sc.turn_speed) * 1000.f;

    // Process Turn Input
    if (key_pressed(KeyboardActions::QuickTurn, keyboard_input) && !quick_turn_active) {
        Entity e = Entity("PlayerQuickTurn");
        PlayerQuickTurn &qt = registry.playerQuickTurn.emplace(e);
        qt.direction = -1;
        qt.duration_ms = quick_turn_duration;
    } else if (!quick_turn_active) {
        float turn_dir = 0.f;
        if (key_held(KeyboardActions::TurnLeft, keyboard_input)) {
            turn_dir -= 1.f;
        }
        if (key_held(KeyboardActions::TurnRight, keyboard_input)) {
            turn_dir += 1.f;
        }
        motion.angle = std::fmod(motion.angle + turn_dir * sc.turn_speed * dt, 2 * M_PI);
    }

    // Process Quick Turn
    if (quick_turn_active)
    {
        assert(registry.playerQuickTurn.size() == 1);
        PlayerQuickTurn &qt = registry.playerQuickTurn.components[0];
        qt.duration_ms -= elapsed_ms;

        float relevant_ms = elapsed_ms;
        if (qt.duration_ms < 0) {
            relevant_ms += qt.duration_ms;
        }

        float quick_turn_amount = (M_PI * relevant_ms * qt.direction) / quick_turn_duration;
        motion.angle = std::fmod(motion.angle + quick_turn_amount, 2 * M_PI);

        if (qt.duration_ms <= 0) {
            for (Entity &e : registry.playerQuickTurn.entities) {
                registry.remove_all_components_of(e);
            }
        }
    }

    if (key_held(KeyboardActions::MoveForward, keyboard_input))
    {
        float max_speed = 3.f * sc.move_speed;
        motion.velocity.x = std::min(max_speed, motion.velocity.x + sc.move_speed * dt);
    } else {
        motion.velocity.x = std::max(0.f, motion.velocity.x - (DECELERATION_MULTIPLIER * sc.move_speed) * dt);
    }

    FireControl &fc = registry.fireControl.get(playerEntity);
    fc.cooldown_timer = std::max(0.f, fc.cooldown_timer - elapsed_ms);

    // ===== Turret aim =====
    vec2 player_pos = motion.position;
    vec2 mouse_relative_pos = mouse_input.mouse_pos - vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
    float aim_angle = atan2(mouse_relative_pos.y, mouse_relative_pos.x);

    if (!registry.turrets.entities.empty())
    {
        Entity turret_entity = registry.turrets.entities[0];
        Motion& tm = registry.motions.get(turret_entity);
        tm.position = player_pos;
        tm.angle = aim_angle;
    }

    if (mouse_input.current_action == MouseActions::LeftClick && fc.cooldown_timer == 0.f)
    {
        // Motion &player_motion = registry.motions.get(registry.players.entities[0]);
        // vec2& player_pos = player_motion.position;

        // // If we wanted to change this so that the player isn't always centered, we would need to add the new 
        // // center coordinates, then subtract the player coordinates. As it stands, since player is the center,
        // // we would be both adding and subtracting the player coordinates.
        // vec2 mouse_relative_pos = mouse_input.mouse_pos - vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
        // // https://stackoverflow.com/questions/2339487/calculate-angle-of-2-points
        // // We assume there is exactly 1 player and 1 turret
        // Entity player = registry.players.entities[0];
        // float barrel_len = 30.f;
        // if (registry.turrets.entities.size() > 0)
        // {
        //     Entity turret_entity = registry.turrets.entities[0];
        //     if (registry.turrets.has(turret_entity))
        //         barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
        // }

        // // bullet spawn at turret tip
        // vec2 forward = { cosf(aim_angle), sinf(aim_angle) };
        // vec2 bullet_spawn_pos = motion.position + forward * barrel_len;

        // createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player);
        // fc.cooldown_timer = fc.cooldown_ms;

        // debug_print_player_motion(motion);
        // debug_print_input_state(keyboard_input);
        float barrel_len = 30.f;
        if (!registry.turrets.entities.empty())
        {
            Entity turret_entity = registry.turrets.entities[0];
            barrel_len = registry.turrets.get(turret_entity).barrel_length_px;
        }

        vec2 forward = { cosf(aim_angle), sinf(aim_angle) };
        vec2 bullet_spawn_pos = motion.position + forward * barrel_len;

        Entity player = registry.players.entities[0];
        createProjectile(bullet_spawn_pos, aim_angle, ATTACK_PATTERN::PLAYER, player);

        fc.cooldown_timer = fc.cooldown_ms;
    }    

    // Shift action over
    std::copy(std::begin(keyboard_input.current_actions), std::end(keyboard_input.current_actions), std::begin(keyboard_input.previous_actions));
}
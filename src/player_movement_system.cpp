// internal
#include "player_movement_system.hpp"
#include <iostream>

float PLAYER_QUICK_TURN_DURATION = 200; // ms

void debug_print_input_state(PlayerInputState &ps) {
    std::cout << "Current:  ";
    for (int i = 0; i < PlayerActionCount; ++i)
    {
        std::cout << ps.current_actions[i] << " ";
    }
    std::cout << "\nPrevious: ";
    for (int i = 0; i < PlayerActionCount; ++i)
    {
        std::cout << ps.previous_actions[i] << " ";
    }
    std::cout << std::endl;
}

void debug_print_player_motion(Motion &m) {
    std::cout << "(" << m.position.x << "," << m.position.y << ") <" << m.velocity.x << "," << m.velocity.y << "> " << m.angle << std::endl;
}

void PlayerMovementSystem::init() {
    Entity e = Entity();
    registry.playerInput.emplace(e);
    // We should have 1 and only ever 1 player input component
    assert(registry.playerInput.components.size() == 1);
}

bool key_pressed(PlayerActions a, const PlayerInputState &ps) {
    return ps.current_actions[a] && !ps.previous_actions[a];
}

bool key_held(PlayerActions a, const PlayerInputState &ps) {
    return ps.current_actions[a];
}

vec2 direction_to_vector(float degree) {
    float radians = degree * (M_PI / 180.f);
    vec2 unit_vec = vec2(
        std::cos(radians),
        std::sin(radians)
    );
    return unit_vec;
}

// TODO: !!! TEMP PROCESS MOVEMENT HERE, PHYSICS SYSTEM SHOULD HANDLE UPDATES
void PlayerMovementSystem::step(float elapsed_ms)
{
    // We should have 1 and only ever 1 player input component
    assert(registry.playerInput.components.size() == 1);
    PlayerInputState &ps = registry.playerInput.components[0];
    Entity playerEntity = registry.players.entities[0];
    Motion &m = registry.motions.get(playerEntity);
    SpeedControl &sc = registry.speedControl.get(playerEntity);

    float dt = elapsed_ms / 1000.0f;
    bool quick_turn_active = (registry.playerQuickTurn.size() != 0);

    // Process Turn Input
    if (key_pressed(PlayerActions::QuickTurn, ps) && !quick_turn_active) {
        Entity e = Entity();
        PlayerQuickTurn &qt = registry.playerQuickTurn.emplace(e);
        qt.direction = -1;
        qt.duration_ms = PLAYER_QUICK_TURN_DURATION;
    } else if (!quick_turn_active) {
        float turn_dir = 0.f;
        if (key_held(PlayerActions::TurnLeft, ps)) {
            turn_dir -= 1.f;
        }
        if (key_held(PlayerActions::TurnRight, ps)) {
            turn_dir += 1.f;
        }
        m.angle = std::fmod(m.angle + turn_dir * sc.turn_speed * dt, 360.f);
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

        float quick_turn_amount = (180.f * relevant_ms * qt.direction) / PLAYER_QUICK_TURN_DURATION;
        m.angle = std::fmod(m.angle + quick_turn_amount, 360.f);

        if (qt.duration_ms <= 0) {
            for (Entity &e : registry.playerQuickTurn.entities) {
                registry.remove_all_components_of(e);
            }
        }
    }

    if (key_held(PlayerActions::MoveForward, ps))
    {
        m.position += direction_to_vector(m.angle) * ((sc.move_speed * elapsed_ms) / 1000.f);
    }

    // debug_print_player_motion(m);
    // debug_print_input_state(ps);

    // Shift action over
    std::copy(std::begin(ps.current_actions), std::end(ps.current_actions), std::begin(ps.previous_actions));
}
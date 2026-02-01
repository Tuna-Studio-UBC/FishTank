// internal
#include "player_movement_system.hpp"
#include <iostream>

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

void PlayerMovementSystem::init()
{
    Entity e = Entity();
    registry.playerInput.emplace(e);
    // We should have 1 and only ever 1 player input component
    assert(registry.playerInput.components.size() == 1);
}

void PlayerMovementSystem::step(float elapsed_ms)
{
    // We should have 1 and only ever 1 player input component
    assert(registry.playerInput.components.size() == 1);
    PlayerInputState &ps = registry.playerInput.components[0];

    // Highest Priority: S
    debug_print_input_state(ps);

    // Shift action over and reset the current actions to false
    std::copy(std::begin(ps.current_actions), std::end(ps.current_actions), std::begin(ps.previous_actions));
}
#include "transition_system.hpp"

#include "states/boss_states.hpp"

void TransitionSystem::step(float elapsed_ms) {
    for (Entity e : registry.stateMachines.entities) {
        StateMachine &fsm = registry.stateMachines.get(e);

        if (!registry.stateTransitionRequests.has(e))
            continue;

        StateTransitionRequest &request = registry.stateTransitionRequests.get(e);
        fsm.change_state(e, create_state(request.next_state));
        registry.stateTransitionRequests.remove(e);
    }
}

std::unique_ptr<State> TransitionSystem::create_state(STATE_TYPE type) {
    switch (type) {
    case STATE_TYPE::PATTERN_A:
        return std::make_unique<PatternAState>();
    case STATE_TYPE::PATTERN_B:
        return std::make_unique<PatternBState>();
    case STATE_TYPE::IDLE:
    default:
        return std::make_unique<IdleState>();
    }
}

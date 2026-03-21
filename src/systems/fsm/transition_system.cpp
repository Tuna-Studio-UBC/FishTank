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
    // Sharky
    case STATE_TYPE::SHARKY_IDLE:
        return std::make_unique<SharkyIdleState>();
    case STATE_TYPE::SHARKY_NORMAL:
        return std::make_unique<SharkyNormalState>();
    case STATE_TYPE::SHARKY_WAVES:
        return std::make_unique<SharkyWavesState>();
    case STATE_TYPE::SHARKY_ENRAGED:
        return std::make_unique<SharkyEnragedState>();

    // King Crabby
    case STATE_TYPE::KING_CRABBY_IDLE:
        return std::make_unique<KingCrabbyIdleState>();
    case STATE_TYPE::KING_CRABBY_NORMAL:
        return std::make_unique<KingCrabbyNormalState>();
    case STATE_TYPE::KING_CRABBY_TOXIC:
        return std::make_unique<KingCrabbyToxicState>();
    case STATE_TYPE::KING_CRABBY_ENRAGED:
        return std::make_unique<KingCrabbyEnragedState>();

    // TODO: EEEEL

    // Idle
    case STATE_TYPE::IDLE:
    default:
        return std::make_unique<IdleState>();
    }
}

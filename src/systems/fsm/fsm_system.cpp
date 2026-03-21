#include "fsm_system.hpp"

#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

void FSMSystem::step(float elapsed_ms) {
    for (Entity entity : registry.stateMachines.entities) {
        StateMachine &fsm = registry.stateMachines.get(entity);
        if (fsm.curr_state)
            fsm.curr_state->update(entity, elapsed_ms);
    }
}

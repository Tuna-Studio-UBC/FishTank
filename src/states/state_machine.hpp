#pragma once

#include "state.hpp"

struct StateMachine {
    std::unique_ptr<State> curr_state;

    void change_state(Entity e, std::unique_ptr<State> new_state) {
        if (curr_state)
            curr_state->on_exit(e);

        curr_state = std::move(new_state);

        if (curr_state)
            curr_state->on_enter(e);
    }
};

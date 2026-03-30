#include "hazard_config.hpp"
#include "common.hpp"

struct HazardConfig {
    HAZARD_TYPE type;
    vec3 colour;
    float amount;
    float duration_ms = 0.f;
    float interval_ms = 0.f;

    /// Set duration_ms for Expiration,
    /// Set duration_ms and interval_ms for OverTimeEffect
    HazardConfig(HAZARD_TYPE type, vec3 colour, float amount, float duration_ms = 0.f, float interval_ms = 0.f) {
        this->type = type;
        this->colour = colour;
        this->amount = amount;
        this->duration_ms = duration_ms;
        this->interval_ms = interval_ms;
    }
};

static const HazardConfig HAZARD_CONFIGS[hazard_count] = {
    HazardConfig(HAZARD_TYPE::MINE, vec3(.2f, .2f, .2f), 50.f),
    HazardConfig(HAZARD_TYPE::ANEMONE, vec3(1.f, .2f, 1.f), 15.f, 5000.f, 1000.f),
    HazardConfig(HAZARD_TYPE::ANEMONE, vec3(1.f, .2f, 1.f), 15.f, 3000.f, 1000.f),
};

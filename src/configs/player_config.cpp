#include <common.hpp>
#include <glm/vec3.hpp>

struct PlayerConfig {
    // Stats
    int hp = 100;
    int max_hp = 100;
    int crackers = 0;

    // Firing
    float fire_rate_cd_ms = 500.f;

    // Movement
    float move_speed = 60;
    float turn_speed = 1.2 * M_PI;

    // Appearance
    vec3 colour = vec3(0.5f, 0.5f, 0.5f);
};

#include "power_up_config.hpp"
#include "../common.hpp"

struct PowerUpConfig
{
  POWER_UP_TYPE type;
  vec3 colour;
  float amount;
  float duration_ms = 0.f;
  float interval_ms = 0.f;

  /// Set duration_ms for Expiration,
  /// Set duration_ms and interval_ms for OverTimeEffect
  PowerUpConfig(POWER_UP_TYPE type, vec3 colour, float amount, float duration_ms = 0.f, float interval_ms = 0.f)
  {
    this->type = type;
    this->colour = colour;
    this->amount = amount;
    this->duration_ms = duration_ms;
    this->interval_ms = interval_ms;
  }
};

static const PowerUpConfig POWER_UP_CONFIGS[power_up_count] = {
  PowerUpConfig(POWER_UP_TYPE::HEAL, vec3(0, 1.f, 0), 50.f),
  PowerUpConfig(POWER_UP_TYPE::SPEED_MULTIPLIER, vec3(0, 1.f, 1.f), 1.5f, 5000.f),
  PowerUpConfig(POWER_UP_TYPE::DAMAGE_MULTIPLIER, vec3(1.f, 1.f, 0), 2.f, 5000.f),
};

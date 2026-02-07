#include <vector>
#include "enemy_config.hpp"
#include <common.hpp>

struct Spawn
{
	ENEMY_TYPE type;
	vec2 position;
};

struct Wave
{
	std::vector<Spawn> enemies;
};

// TODO: Position coordinates in world space instead of screen space
const std::vector<Wave> WAVE_CONFIGS = {
	{
		{ {ENEMY_TYPE::FOLLOWER, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 5 }} }
	},
	{
		{ {ENEMY_TYPE::FOLLOWER, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 5 }},
		{ENEMY_TYPE::URCHIN, vec2{ TILE_SIZE_PX * 10, TILE_SIZE_PX * 10 }}}
	},
	{
		{ {ENEMY_TYPE::SWARM, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 5 }},
		{ENEMY_TYPE::SWARM, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 10 }},
		{ENEMY_TYPE::SWARM, vec2{ TILE_SIZE_PX * 10, TILE_SIZE_PX * 5 }},
		{ENEMY_TYPE::SWARM, vec2{ TILE_SIZE_PX * 5, TILE_SIZE_PX * 10 }}}
	}
};
#include <iostream>

#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "configs/enemy_config.cpp"
#include "configs/attack_pattern.cpp"

// #define DEBUG_INIT_TILES

#ifdef DEBUG_INIT_TILES
void print_tilemap(TileMap &tilemap)
{
	std::cout << "TILEMAP: \n[\n";
	for (int y = 0; y < tilemap.height; y++)
	{
		std::cout << "[";
		for (int x = 0; x < tilemap.width; x++)
		{
			std::cout << tilemap.tile_ids[x + tilemap.width * y] << ", ";
		}
		std::cout << "]\n";
	}
	std::cout << "]\n";
}

void print_tileset(TileSet &tileset)
{
	std::cout << "TILESET: \n[\n";
	for (int y = 0; y < tileset.dimensions.y; y++)
	{
		std::cout << "[";
		for (int x = 0; x < tileset.dimensions.x; x++)
		{
			std::cout << (int)tileset.collisions[x + tileset.dimensions.x * y] << ", ";
		}
		std::cout << "]\n";
	}
	std::cout << "]\n";
}
#endif

void init_tiles()
{
	Entity entity = Entity();

	TileMap tilemap = parse_tilemap(TILEMAP_JSON);
	TileSet tileset = parse_tileset(TILESET_JSON);

	registry.tilemaps.insert(entity, tilemap);
	registry.tilesets.insert(entity, tileset);

	registry.renderRequests.insert(
		entity,
		{(TEXTURE_ASSET_ID)tilemap.tileset_id,
		 EFFECT_ASSET_ID::TILEMAP,
		 GEOMETRY_BUFFER_ID::SPRITE});

#ifdef DEBUG_INIT_TILES
	print_tilemap(tilemap);
	print_tileset(tileset);
#endif
}

#pragma region Actors
// =================================================================================
// Actors
// =================================================================================

// Player creation function
Entity createPlayer(RenderSystem *renderer, vec2 position, int health, int max_health)
{
	auto entity = Entity();

	registry.players.emplace(entity);

	registry.keyboardInput.emplace(entity);
	registry.mouseInput.emplace(entity);

	Wallet &w = registry.wallet.emplace(entity);
	w.crackers = 0; // !!! Hardcode for now

	Health &h = registry.health.emplace(entity);
	h.hp = health;
	h.max_hp = max_health;

	FireControl &fc = registry.fireControl.emplace(entity);
	fc.cooldown_ms = 500; // !!! Hardcode for now
	
	SpeedControl &sc = registry.speedControl.emplace(entity);
	sc.move_speed = 60; // !!! Hardcode for now
	sc.turn_speed = 1.2 * M_PI; // !!! Hardcode for now
	
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
	registry.meshPtrs.emplace(entity, &mesh);
	
	Motion &motion = registry.motions.emplace(entity);
	motion.velocity = vec2(0.0f, 0.0f);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2(50.0f, 50.0f);

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	registry.colors.emplace(entity, vec3(0.5f, 0.5f, 0.5f)); // Gray color

	return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, ENEMY_TYPE type)
{
	auto entity = Entity();

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &config = ENEMY_CONFIGS[(int)type];

	auto &enemy = registry.enemies.emplace(entity);
	enemy.detection_radius = config.detection_radius;

	auto &motion = registry.motions.emplace(entity);
	motion.velocity = vec2(0.0f, 0.0f);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2(50.0f, 50.0f);

	registry.health.emplace(entity);

	auto &fire = registry.fireControl.emplace(entity);
	fire.attack_pattern = config.attack_pattern;

	auto &speed = registry.speedControl.emplace(entity);
	speed.move_speed = config.move_speed;
	speed.turn_speed = config.turn_speed;

	registry.dropsCrackers.emplace(entity);

	if (type == ENEMY_TYPE::PATROLLER)
	{
		registry.patrolling.emplace(entity);
	}

	registry.colors.emplace(entity, vec3(1.0f, 0.2f, 0.2f));
	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::EGG,
			 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	return entity;
}

/*Entity createEnemy(RenderSystem* renderer, vec2 position, vec2 patrol_a, vec2 patrol_b, float speed_px_s)
{
    auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
    registry.meshPtrs.emplace(entity, &mesh);

    auto& motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(45.0f, 45.0f);

    registry.colors.emplace(entity, vec3(1.0f, 0.2f, 0.2f)); 

    registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::TEXTURE_COUNT,
          EFFECT_ASSET_ID::EGG,
          GEOMETRY_BUFFER_ID::DEBUG_LINE });

    registry.enemies.emplace(entity);

	Patrol patrol;
	patrol.speed_px_s = speed_px_s;
	patrol.change_dir_min_ms = 3000.f;
	patrol.change_dir_max_ms = 7000.f;
	patrol.time_to_change_ms = 0.f;
	patrol.dir = { 1.f, 0.f };
	patrol.padding_px = 5.f;

	registry.patrols.insert(entity, patrol);

    return entity;
}*/

#pragma endregion Actors

#pragma region Pickups
// =================================================================================
// Pickups
// =================================================================================



#pragma endregion Pickups

#pragma region Distractions
// =================================================================================
// Distractions
// =================================================================================

Entity createBell(RenderSystem* renderer, vec2 position, float interact_radius_px)
{
    auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EGG);
    registry.meshPtrs.emplace(entity, &mesh);

    auto& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {30.f, 30.f};

    registry.colors.emplace(entity, vec3(0.1f, 0.9f, 0.1f));
    registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::TEXTURE_COUNT,
          EFFECT_ASSET_ID::EGG,
          GEOMETRY_BUFFER_ID::EGG });

    DistractionTrigger trig;
    trig.interact_radius_px = interact_radius_px;
    trig.cooldown_ms = 800.f;
    trig.cooldown_remaining_ms = 0.f;
    registry.distractionTriggers.insert(entity, trig);

    return entity;
}

#pragma endregion Distractions

#pragma region Hazards
// =================================================================================
// Hazards
// =================================================================================

Entity createProjectile(vec2 position, float angle, ATTACK_PATTERN attack_pattern, Entity owner)
{
	auto entity = Entity();

	AttackConfig attack_config = ATTACK_CONFIGS[(int)attack_pattern];

	auto &bullet = registry.bullet.emplace(entity);
	bullet.damage = attack_config.damage;
	bullet.owner = owner;
  bullet.lifespan_ms = attack_config.lifespan_ms;

	auto &motion = registry.motions.emplace(entity);
	motion.velocity = vec2(attack_config.move_speed, 0.f);
	motion.position = position;
	motion.angle = angle;
	motion.scale = vec2(PROJECTILE_SIZE_PX);

	registry.colors.emplace(entity, vec3(0.5f, 0.5f, 0.5f)); // Gray color
	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::TEXTURE_PROJECTILE,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});
	return entity;
}

#pragma endregion Hazards

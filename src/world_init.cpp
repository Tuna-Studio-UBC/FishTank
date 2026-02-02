#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

#define DEBUG_INIT_TILES

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

// Player creation function
Entity createPlayer(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	registry.players.emplace(entity);

	Wallet &w = registry.wallet.emplace(entity);
	w.crackers = 0; // !!! Hardcode for now

	Health &h = registry.health.emplace(entity);
	h.hp = 200; // !!! Hardcode for now
	h.max_hp = 200; // !!! Hardcode for now

	FireControl &fc = registry.fireControl.emplace(entity);
	fc.cooldown_ms = 2000; // !!! Hardcode for now
	
	SpeedControl &sc = registry.speedControl.emplace(entity);
	sc.move_speed = 60; // !!! Hardcode for now
	sc.turn_speed = 250; // !!! Hardcode for now

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

// Enemy creation function
Entity createEnemy(RenderSystem* renderer, vec2 position, float radius) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& deadly = registry.deadlys.emplace(entity);
	deadly.radius = radius;

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = vec2(0.0f, 0.0f);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2(50.0f, 50.0f);
	registry.colors.emplace(entity, vec3(0.5f, 0.5f, 0.5f)); // Gray color
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	return entity;
}
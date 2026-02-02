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

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.velocity = vec2(0.0f, 0.0f);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2(50.0f, 50.0f);
	registry.colors.emplace(entity, vec3(0.5f, 0.5f, 0.5f)); // Gray color
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE});
	registry.players.emplace(entity);
	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position, vec2 patrol_a, vec2 patrol_b, float speed_px_s)
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
}

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

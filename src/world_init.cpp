#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

#define DEBUG_INIT_TILES

#ifdef DEBUG_INIT_TILES
void print_tilemap(TileMap& tilemap)
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

void print_tileset(TileSet& tileset)
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
		{
			(TEXTURE_ASSET_ID)tilemap.tileset_id,
			EFFECT_ASSET_ID::TILEMAP,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

#ifdef DEBUG_INIT_TILES
	print_tilemap(tilemap);
	print_tileset(tileset);
#endif
}
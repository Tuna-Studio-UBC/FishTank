#include "parse_tiled.hpp"

std::string parse_file(const std::string &path) {
	std::string line, text = "";
	std::ifstream ifs(path);
	if (ifs.is_open())
	{
		while (std::getline(ifs, line))
		{
			text += line + "\n";
		}
	}
	else
	{
		std::cout << "ERROR: Could not parse file" << std::endl;
	}
	return text;
}

TileMap parse_tilemap(const std::string& path)
{
	auto json_str = parse_file(path);
	rapidjson::Document map_dom;
	map_dom.Parse(json_str.c_str());

	auto layer_array = map_dom["layers"].GetArray();
	assert(layer_array.Size() > 0 && "Map must have at least one layer!");

	// TODO: Fix this, it should find the main layer?
	rapidjson::Value &main_layer = layer_array[0];

	const int width = main_layer["width"].GetInt();
	const int height = main_layer["height"].GetInt();
	const auto tile_data = main_layer["data"].GetArray();


	std::array<unsigned short, MAX_TILEMAP_SIZE> tiles{0};
	for (int i = 0; i < width * height; i++)
	{
		tiles[i] = tile_data[i].GetInt();
	}

	return TileMap(tiles, width, height);
}

TileSet parse_tileset(const std::string& path)
{
	auto json_str = parse_file(path);
	rapidjson::Document tileset_dom;
	tileset_dom.Parse(json_str.c_str());

	int width   = tileset_dom["columns"  ].GetInt();
	int n_tiles = tileset_dom["tilecount"].GetInt();
	int height  = width / n_tiles;
	if (width % n_tiles != 0)
		height++;

	int height_px = tileset_dom["tileheight"].GetInt();
	int width_px  = tileset_dom["tilewidth" ].GetInt();

	auto tiles = tileset_dom["tiles"].GetArray();

	std::array<bool, MAX_TILESET_SIZE> collisions{0};
	for (int i = 0; i < n_tiles; i++) 
	{
		auto obj = tiles[i].GetObject();
		auto properties = obj["properties"].GetArray();
		collisions[i] = properties[0]["value"].GetBool();
	}

	return TileSet(width, height, width_px, height_px, collisions);
}
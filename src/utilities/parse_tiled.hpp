#pragma once
#include "../../ext/rapidjson/document.h"
#include "tinyECS/components.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>

std::string parse_file(const std::string &path);

rapidjson::Value *get_layer(std::string layer_name, rapidjson::Document &map_dom);

TileMap parse_tilemap_layer(const std::string &path, const std::string &layer, const bool &get_spawn_points = false);
TileMap parse_tilemap(const std::string &path);
TileSet parse_tileset(const std::string &path);

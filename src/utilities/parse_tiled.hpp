#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <array>
#include "../tinyECS/components.hpp"
#include "../ext/rapidjson/document.h"

std::string parse_file(const std::string &path);

TileMap parse_tilemap(const std::string& path);

TileSet parse_tileset(const std::string& path);
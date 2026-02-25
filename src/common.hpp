#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
// clang-format off
#include <gl3w.h>
#include <GLFW/glfw3.h>
// clang-format on

// The glm library provides vector and matrix operations as in GLSL
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/mat3x3.hpp>          // mat3
#include <glm/vec2.hpp>            // vec2
#include <glm/vec3.hpp>            // vec3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string &name) {
    return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;
};
inline std::string textures_path(const std::string &name) {
    return std::string(PROJECT_SOURCE_DIR) + "res/textures/" + std::string(name);
};
inline std::string tileset_texture_path(const std::string &name) {
    return std::string(PROJECT_SOURCE_DIR) + "res/tileset_images/" + std::string(name);
};
inline std::string sprite_sheet_texture_path(const std::string &name) {
    return std::string(PROJECT_SOURCE_DIR) + "res/sprite_sheet/" + std::string(name);
};
inline std::string audio_path(const std::string &name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string &name) { return data_path() + "/meshes/" + std::string(name); };

//
// Define screen sizes
//
inline std::unordered_map<std::string, std::pair<int, int>> SCREEN_SIZES = {{"default", std::make_pair(800, 600)},
                                                                            {"hd", std::make_pair(1280, 720)},
                                                                            {"widescreen", std::make_pair(1440, 900)},
                                                                            {"fhd", std::make_pair(1920, 1080)},
                                                                            {"qhd", std::make_pair(2560, 1440)}};

//
// game constants
//
inline const char *GAME_NAME = "FishTank!";

inline const std::string WINDOW_SIZE = "hd";
const int WINDOW_WIDTH_PX = SCREEN_SIZES[WINDOW_SIZE].first;
const int WINDOW_HEIGHT_PX = SCREEN_SIZES[WINDOW_SIZE].second;

const int TILE_SIZE_PX = 50;
const int TILE_WIDTH_PX = TILE_SIZE_PX;
const int TILE_HEIGHT_PX = TILE_SIZE_PX;

constexpr size_t MAX_TILEMAP_SIZE = 4096;
constexpr size_t MAX_TILESET_SIZE = 2048;

const int PROJECTILE_SPEED = 200;
const int PROJECTILE_SIZE_PX = 20;

// player constants
const int STARTING_AMMO = 30;
const int PROGRESS_PER_AMMO = 1000;

// passive constants

// enemy constants

// object constants

// These are hard coded to the dimensions of the entity's texture

// bounding boxes
const float _BB_WIDTH = (float)TILE_WIDTH_PX;
const float _BB_HEIGHT = (float)TILE_HEIGHT_PX;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
    mat3 mat = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}; // start with the identity
    void scale(vec2 scale);
    void rotate(float radians);
    void translate(vec2 offset);
};

bool gl_has_errors();

vec2 direction_to_unit_vector(float radians);
void change_vector_direction(vec2 &in_vec, float radians);
bool entity_in_radius(vec2 dist, vec2 entity_scale, float radius);

ivec2 world_to_tile(const vec2 &world_pos);

vec2 tile_center_to_world(const ivec2 &tile);

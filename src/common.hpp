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

// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"

// DEBUG GOD MODE
// #define GOD_MODE

//
// Define directories
//
inline const std::string ROOT_DIR = std::string(PROJECT_SOURCE_DIR);

enum FilePath {
    AUDIO,
    DECISION_TREES,
    FONTS,
    MAPS,
    MESHES,
    SHADERS,
    SPRITE_SHEETS,
    TEXTURES,
    TILESETS,
    TILESET_IMAGES,
    UI_DOCUMENTS,
    UI_STYLES
};

inline std::string get_path(FilePath path, const std::string &file_name) {
    std::string file = std::string(file_name);
    switch (path) {
    case AUDIO:
        return ROOT_DIR + "/res/audio/" + file;
    case DECISION_TREES:
        return ROOT_DIR + "/res/decision_trees/" + file;
    case FONTS:
        return ROOT_DIR + "/res/fonts/" + file;
    case MAPS:
        return ROOT_DIR + "/res/maps/" + file;
    case MESHES:
        return ROOT_DIR + "/res/meshes/" + file;
    case SHADERS:
        return ROOT_DIR + "/shaders/" + file;
    case SPRITE_SHEETS:
        return ROOT_DIR + "/res/sprite_sheets/" + file;
    case TEXTURES:
        return ROOT_DIR + "/res/textures/" + file;
    case TILESETS:
        return ROOT_DIR + "/res/tilesets/" + file;
    case TILESET_IMAGES:
        return ROOT_DIR + "/res/tileset_images/" + file;
    case UI_DOCUMENTS:
        return ROOT_DIR + "/res/ui/documents/" + file;
    case UI_STYLES:
        return ROOT_DIR + "/res/ui/styles/" + file;
    default:
        return "/";
    }
};

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

const int BASE_PROJECTILE_SPEED = 200;
const int PROJECTILE_SIZE_PX = 20;

inline const bool is_empty_tile(int tile_id) { return (tile_id == 0 || tile_id == 1); }

// player constants
const int STARTING_AMMO = 40;
const int PROGRESS_PER_AMMO = 1000;

const vec2 UNSET_POS = vec2(-999999.f, -999999.f);

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
vec2 get_vector_rotated(vec2 &in_vec, float radians);
bool entity_in_radius(vec2 dist, vec2 entity_scale, float radius);

ivec2 world_to_tile(const vec2 &world_pos);
int get_map_index(int width, int x, int y);
vec2 tile_center_to_world(const ivec2 &tile);
float get_random_float(float min, float max);

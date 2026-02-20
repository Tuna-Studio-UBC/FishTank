#pragma once
#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include "configs/attack_pattern.hpp"
#include "configs/hazard_config.cpp"
#include "configs/power_up_config.cpp"
#include <array>
#include <unordered_map>
#include <vector>

#pragma region Entity
// =================================================================================
// Entity (Represents entity, loop through list to get all)
// =================================================================================

struct Player {};
// Uses: Motion, Health, FireControl, SpeedControl, Wallet

struct Enemy {
    float detection_radius;
};
// Uses: Motion, Health, FireControl, SpeedControl, DropsCrackers
// Optional: Patrolling, Pursuit

struct Bullet {
    int damage;
    Entity owner; // track who fired to prevent self hit
};
// Uses: Motion
// Optional: Bouncy

struct Turret {
    float barrel_length_px = 30.f;
};

struct Bell {
    float interact_radius_px = 80.f; // distance to e
    float distraction_radius = 200.f;
    float cooldown_ms = 800.f;
    float cooldown_timer = 0.f;
    float distraction_ms = 5000.f;
};

struct Wall {};
// Uses: Motion

struct Camera {};
// Uses: Motion

#pragma endregion Entity

#pragma region Hazards
// =================================================================================
// Hazards
// =================================================================================

struct Hazard {
    HAZARD_TYPE type;
    Hazard(HAZARD_TYPE type) { this->type = type; };
};

struct Mine {};
// Uses: Motion

struct Anemone {};
// Uses: Motion

#pragma endregion Hazards

#pragma region Behaviour
// =================================================================================
// Behaviour (Attached to entities, e.g. Bouncy, Health, ...)
// =================================================================================

struct Health {
    int max_hp = 100;
    int hp = 100;
};

struct Dead {};

struct Damage {
    int amount;
    Damage(int amount) { this->amount = amount; };
};
// Optional: OverTimeEffect

struct Expiration {
    float lifespan_ms = 5000; // ms
    Expiration(float lifespan_ms) { this->lifespan_ms = lifespan_ms; };
};

struct OverTimeEffect {
    float interval_ms = 1000;      // ms
    float duration_time_ms = 3000; // ms
    float elapsed_time_ms = 0;     // ms
    OverTimeEffect(float interval_ms, float duration_time_ms) {
        this->interval_ms = interval_ms;
        this->duration_time_ms = duration_time_ms;
    };
};

struct FireControl {
    float cooldown_timer = 0; // ms
    float cooldown_ms = 2000; // ms
    ATTACK_PATTERN attack_pattern;
};

struct SpeedControl {
    float move_speed = 0;
    float turn_speed = 0;
};

struct PlayerQuickTurn {
    float duration_ms;
    float direction; // -1.f for left, 1.f for right
};

struct Target {
    Entity target;
    Target(Entity &target) { this->target = target; };
};

struct DropsCrackers {
    int cracker;
};

struct Wallet {
    int crackers = 0;
};
// Uses: Motion

struct Patrolling {};

struct Pursuit {};

struct Bouncy {
    int bounces_remaining = 3;
};

struct Distracted {
    Entity bell;
    float remaining_ms;
};

// used in AISystem to request projectile creation in WorldSystem
struct BulletRequest {
    ATTACK_PATTERN attack_pattern;
    vec2 position;
    float angle;
    Entity owner;
};

#pragma endregion Behaviour

#pragma region Pickups
// =================================================================================
// Pickups
// =================================================================================

struct PickUp {};

struct PowerUp {
    POWER_UP_TYPE type;
    PowerUp(POWER_UP_TYPE type) { this->type = type; };
};

struct Heal {
    int amount;
    Heal(int amount) { this->amount = amount; };
};
// Optional: OverTimeEffect

struct SpeedMultiplier {
    float multiplier;
    SpeedMultiplier(float multiplier) { this->multiplier = multiplier; };
};

struct DamageMultiplier {
    float multiplier;
    DamageMultiplier(float multiplier) { this->multiplier = multiplier; };
};

#pragma endregion Pickups

#pragma region Physics
// =================================================================================
// Physics (Motion, collision, etc)
// =================================================================================

struct Motion {
    vec2 position = {0, 0}; // WORLD COORDINATE px
    float angle = 0;        // Angle in RADIANS
    vec2 velocity = {0, 0}; // px / s
    vec2 scale = {10, 10};  // px
};

// Stucture to store collision information
struct Collision {
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
};

struct ParticleEmitter {
    vec2 position; // Position of the particle spawn
    int max_particles;
    float particle_speed;    // Speed of particle in px / s
    float spawn_interval_ms; // Minimum ms between particle spawns
    float life_ms;           // Duration of particle spawns
    float particle_life_ms;  // Duration of each particle
};

#pragma endregion Physics

#pragma region Game State
// =================================================================================
// Game State
// =================================================================================

// Data structure for toggling debug mode
struct Debug {
    bool in_debug_mode = 0;
    bool in_freeze_mode = 0;
};
extern Debug debugging;

struct ScreenState {
    float darken_screen_factor = -1;
};

enum KeyboardActions { MoveForward, TurnLeft, TurnRight, QuickTurn, KeyboardActionsCount };

struct KeyboardInputState {
    bool current_actions[KeyboardActions::KeyboardActionsCount];
    bool previous_actions[KeyboardActions::KeyboardActionsCount];
};

enum MouseActions { LeftClick, MouseActionsCount };

struct MouseInputState {
    vec2 mouse_pos;
    MouseActions current_action = MouseActionsCount;
};

struct Progress {
    int wave = 0;
    int level = 0;
    int remaining_enemies = -1;
    bool game_over = false;
};

#pragma endregion Game State

#pragma region Rendering
// =================================================================================
// Rendering
// =================================================================================

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex {
    vec3 position;
    vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
    vec3 position;
    vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh {
    static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices,
                                std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
    vec2 original_size = {1, 1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
};

#define GET_2D_POS(x, y, width) x + width *y

struct TileMap {
    int tileset_id = 0; // TODO: Add ID functionality
    std::array<unsigned short, 4096> tile_ids;
    unsigned int width;
    unsigned int height;

    TileMap(std::array<unsigned short, 4096> tile_ids, unsigned int width, unsigned int height)
        : tile_ids(tile_ids), width(width), height(height) {}

    TileMap(const TileMap &other) {
        this->tile_ids = other.tile_ids;
        this->width = other.width;
        this->height = other.height;
    }
};

struct TileSet {
    int tileset_id = 1; // TODO: Add ID functionality
    vec2 dimensions;
    int tile_width_px;
    int tile_height_px;
    std::array<bool, 2048> collisions;

    TileSet(int width, int height, int tile_width_px, int tile_height_px, std::array<bool, 2048> collisions)
        : dimensions(vec2(width, height)), tile_width_px(tile_width_px), tile_height_px(tile_height_px),
          collisions(collisions) {}
};

struct Overlay {};

struct Animation {
    int current_frame = 0;
    int frame_count;            // Info for looping, useful also if count != cols x rows
    float frame_time = 1000;    // ms for frame transition, defaults to 1000
    float accumulator = 0;
    int columns;                // Columns in sprite sheet
    int rows = 1;               // Rows in sprite sheet
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
    TEXTURE_TILESET = 0,
    TEXTURE_PROJECTILE = 1,
    TEXTURE_BLACK = 2,
    TEXTURE_WIN_OVERLAY = 3,
    TEXTURE_LOSE_OVERLAY = 4,
    TEXTURE_BUBBLE = 5,
    TEXTURE_BUBBLE_SHEET = 6,
    TEXTURE_COUNT = TEXTURE_BUBBLE_SHEET + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    COLOURED = 0,
    EGG = COLOURED + 1,
    CHICKEN = EGG + 1,
    TEXTURED = CHICKEN + 1,
    VIGNETTE = TEXTURED + 1,
    TILEMAP = VIGNETTE + 1,
    PARTICLE = TILEMAP + 1,
    EFFECT_COUNT = PARTICLE + 1,
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    CHICKEN = 0,
    SPRITE = CHICKEN + 1,
    EGG = SPRITE + 1,
    DEBUG_LINE = EGG + 1,
    SCREEN_TRIANGLE = DEBUG_LINE + 1,
    GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};

const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

#pragma endregion Rendering

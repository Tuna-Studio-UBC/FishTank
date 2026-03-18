#pragma once
#include <array>
#include <unordered_map>
#include <vector>

#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include "configs/attack_pattern.hpp"
#include "configs/hazard_config.cpp"
#include "configs/power_up_config.cpp"
#include "configs/spawn_target_config.hpp"
#include "configs/upgrade_config.hpp"
#include <array>
#include <unordered_map>
#include <utilities/parse_tree.hpp>

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
    int ammo = 0;
    int max_ammo = STARTING_AMMO;
    int damage_bonus = 0;
    float refill_progress = 0.f;
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
    unsigned int crackers = 0;
};
// Uses: Motion

struct Pursuit {
    std::vector<vec2> path;
    size_t curr_index = 0;
    ivec2 goal;
};

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

struct Bezier {
    int current_segment = 0;
    float t = 0.f;
    float speed = 0.15f;
    vec2 velocity;
};

struct Leader {
    int followers = 0;
};

struct Boid {
    float perception_radius = 100.f;
    float alignment_weight = 1.f;
    float cohesian_weight = 0.9f;
    float separation_weight = 1.5f;
    vec2 velocity;
    Entity leader;
};

struct Sense {
    bool detects_player = false;
    bool is_distracted = false;
    bool is_patrolling = true;
    vec2 last_known_player_pos;
};

struct AI {
    std::string tree_file;
    ScriptedTree tree;
    AIAction current_state = AIAction::IDLE;
};

struct Upgrades {
    UpgradeInstance upgrade_instances[UPGRADE_TYPE::UPGRADE_COUNT] = {UpgradeInstance(ATTACK_UPGRADE_CONFIG),
                                                                      UpgradeInstance(MAX_AMMO_UPGRADE_CONFIG),
                                                                      UpgradeInstance(MAX_HP_UPGRADE_CONFIG)};
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

struct Bubbler {
    float ammo_per_second = 10.f;
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
    vec2 position = {0, 0};     // WORLD COORDINATE px
    float angle = 0;            // Angle in RADIANS
    float angular_velocity = 0; // Also in RADIANS / s
    vec2 velocity = {0, 0};     // px / s
    vec2 scale = {10, 10};      // px
    // Last known non-colliding position (used as a robust fallback recovery)
    vec2 last_safe_position = {0, 0};

    // Contact caching to stabilize repeated small MTV corrections:
    // last contact normal (world-space), and frames to keep it active
    vec2 contact_normal = {0, 0};
    int contact_cooldown_frames = 0;
};

// Stucture to store collision information
struct Collision {
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
};
struct MeshCollider {
    std::vector<vec2> vertices;
    float authoring_px = 128.0f; // 128 authoring pixel.
};
// Forward definition to avoid compilation issues, this is defined further down
enum class TEXTURE_ASSET_ID;

// Entities with this require the motion component
struct ParticleEmitter {
    vec2 offset = {0, 0}; // Offset of particles spawns w.r.t. to entity pos and rotation (x is forward)
    int max_particles;
    TEXTURE_ASSET_ID texture;
    float particle_scale = 10.f;
    float particle_speed;    // Speed of particle in px / s
    float spawn_interval_ms; // Minimum ms between particle spawns
    float particle_life_ms;  // Duration of each particle
    float life_ms = -99.f;   // Default -99 (infinite). Duration of particle spawns.
    bool standalone = false; // Default false. True iff this entity is only PE, everything deleted when PE expires
};

#pragma endregion Physics

#pragma region Game State
// =================================================================================
// Game State
// =================================================================================

// Data structure for toggling debug mode
struct Debug {
    bool in_debug_mode = 0; // if 1 render spatial grid
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
    KeyboardActions most_recent_turn = KeyboardActions::TurnRight;
};

enum MouseActions { LeftClick, MouseActionsCount };

struct MouseInputState {
    vec2 mouse_pos;
    MouseActions current_action = MouseActionsCount;
};

// TODO: Move to config
enum Tutorial { MOVEMENT = 1, HAZARDS, HEALING, AMMUNITION, COMBAT, UPGRADE_SYSTEM };

struct TutorialStep {
    Tutorial id;
    int current_step = 1;
    int total_steps; // TODO: Store event triggers instead
    TutorialStep(Tutorial id, int total_steps) : id(id), total_steps(total_steps) {}
};

const std::vector<TutorialStep> TUTORIAL_STEPS = {
    TutorialStep(Tutorial::MOVEMENT, 2),   TutorialStep(Tutorial::HAZARDS, 1), TutorialStep(Tutorial::HEALING, 1),
    TutorialStep(Tutorial::AMMUNITION, 1), TutorialStep(Tutorial::COMBAT, 2),
    TutorialStep(Tutorial::UPGRADE_SYSTEM, 1),
};

struct Progress {
    int wave = 0;
    int level = 0;
    int remaining_enemies = -1;

    // Game state - Tutorial
    bool is_tutorial = false;
    bool is_tutorial_step_called = false;
    bool is_upgrade_menu_called = false;
    Tutorial tutorial_step_id = Tutorial::MOVEMENT;
    std::vector<TutorialStep> tutorial_steps;

    // Game state - Main
    bool is_paused = false;
    bool is_game_over = false;
    bool has_beat_level = false;
};

struct TutorialTrigger {
    Tutorial id;
    TutorialTrigger(Tutorial id) : id(id) {};
};
#pragma endregion Game State

#pragma region Sound
// =================================================================================
// Sound Effect
// =================================================================================
enum class SOUND_EVENT {
    PLAYER_SHOT,
};

struct SoundRequest {
    SOUND_EVENT type;
};

#pragma endregion Sound

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

#define MAX_TILES 4096 // 64x64

struct BezierPath {
    vec2 p0;
    vec2 p1;
    vec2 p2;
    vec2 p3;
};

struct TileMap {
    std::array<unsigned short, MAX_TILES> tile_ids;
    std::array<SPAWN_TARGET, MAX_TILES> spawn_tile_ids;
    unsigned int width;
    unsigned int height;
    unsigned int x_offset = 0;
    unsigned int y_offset = 0;

    std::vector<BezierPath> patrol_loop;

    TileMap(std::array<unsigned short, MAX_TILES> tile_ids, std::array<unsigned int, MAX_TILES> spawn_tile_ids,
            unsigned int width, unsigned int height, unsigned int x_offset = 0, unsigned int y_offset = 0)
        : tile_ids(tile_ids), spawn_tile_ids(spawn_tile_ids), width(width), height(height), x_offset(x_offset),
          y_offset(y_offset) {}

    TileMap(const TileMap &other) {
        this->tile_ids = other.tile_ids;
        this->spawn_tile_ids = other.spawn_tile_ids;
        this->width = other.width;
        this->height = other.height;
        this->x_offset = other.x_offset;
        this->y_offset = other.y_offset;
    }
};

struct TileSet {
    vec2 dimensions;
    int tile_width_px;
    int tile_height_px;
    std::array<bool, 2048> collisions;

    TileSet(int width, int height, int tile_width_px, int tile_height_px, std::array<bool, 2048> collisions)
        : dimensions(vec2(width, height)), tile_width_px(tile_width_px), tile_height_px(tile_height_px),
          collisions(collisions) {}
};

struct Colour {
    vec3 colour = {1.f, 1.f, 1.f};
    Colour(vec3 colour) : colour(colour) {}
};

struct Tint {
    vec3 tint = {1.f, 1.f, 1.f};
    float life_ms = -99.f; // Default -99 (infinite)
    Tint(vec3 tint, float life_ms = -99.f) : tint(tint), life_ms(life_ms) {}
};

struct Text {
    std::string text = "";
    vec2 position = {0.f, 0.f};
    float scale = 1.f;
    vec3 color = {0.f, 0.f, 0.f};
    glm::mat4 trans = glm::mat4(1.f);
};

struct Overlay {};

struct Animation {
    int current_frame = 0;
    int frame_count;         // Info for looping, useful also if count != cols x rows
    float frame_time = 1000; // ms for frame transition, defaults to 1000
    float accumulator = 0;
    int columns;  // Columns in sprite sheet
    int rows = 1; // Rows in sprite sheet
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
    TEXTURE_GREYBOX_TILESET = 0,
    TEXTURE_GREYBOX_TILESET2 = TEXTURE_GREYBOX_TILESET + 1,

    TEXTURE_PROJECTILE = TEXTURE_GREYBOX_TILESET2 + 1,
    TEXTURE_BLACK = TEXTURE_PROJECTILE + 1,
    TEXTURE_TUTORIAL_COMPLETE_OVERLAY = TEXTURE_BLACK + 1,
    TEXTURE_WIN_OVERLAY = TEXTURE_TUTORIAL_COMPLETE_OVERLAY + 1,
    TEXTURE_LOSE_OVERLAY = TEXTURE_WIN_OVERLAY + 1,
    TEXTURE_BUBBLE = TEXTURE_LOSE_OVERLAY + 1,
    TEXTURE_BUBBLER = TEXTURE_BUBBLE + 1,

    TEXTURE_CURSOR = TEXTURE_BUBBLER + 1,

    TEXTURE_SMOKE = TEXTURE_CURSOR + 1,

    TEXTURE_BUBBLE_SHEET = TEXTURE_SMOKE + 1,
    TEXTURE_FISH_SHEET = TEXTURE_BUBBLE_SHEET + 1,
    TEXTURE_TURRET_SHEET = TEXTURE_FISH_SHEET + 1,
    TEXTURE_CRAB_SHEET = TEXTURE_TURRET_SHEET + 1,
    TEXTURE_HEALTH_SHEET = TEXTURE_CRAB_SHEET + 1,
    TEXTURE_MINE_SHEET = TEXTURE_HEALTH_SHEET + 1,
    TEXTURE_URCHIN_SHEET = TEXTURE_MINE_SHEET + 1,
    TEXTURE_ISOPOD_SHEET = TEXTURE_URCHIN_SHEET + 1,

    TEXTURE_COUNT = TEXTURE_ISOPOD_SHEET + 1
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;
const int last_tileset_id = (int)TEXTURE_ASSET_ID::TEXTURE_GREYBOX_TILESET2;

enum class EFFECT_ASSET_ID {
    COLOURED = 0,
    EGG = COLOURED + 1,
    CHICKEN = EGG + 1,
    TEXTURED = CHICKEN + 1,
    FONT = TEXTURED + 1,
    VIGNETTE = FONT + 1,
    TILEMAP = VIGNETTE + 1,
    PARTICLE = TILEMAP + 1,
    EFFECT_COUNT = PARTICLE + 1
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

enum RENDER_LAYER {
    BACKGROUND = 0,
    INTERACTABLES = 5,
    ACTORS = 10,
    PROJECTILES = ACTORS + 1,
    TURRETS = PROJECTILES + 1,
    WALLS = 20,
    OVERLAYS = 100,
};

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
    int layer = RENDER_LAYER::BACKGROUND; // Larger numbers are drawn on top
};

typedef std::pair<Entity, RenderRequest> EntityRRPair;
struct EntityRRPairCompare {
    bool operator()(const EntityRRPair &a, const EntityRRPair &b) const { return a.second.layer > b.second.layer; }
};

#pragma endregion Rendering

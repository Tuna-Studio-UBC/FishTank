#include "world_init.hpp"
#include "configs/attack_pattern.cpp"
#include "configs/enemy_config.cpp"
#include "configs/player_config.cpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <map>
#include <set>

// #define DEBUG_INIT_TILES

#ifdef DEBUG_INIT_TILES
void print_tilemap(TileMap &tilemap) {
    std::cout << "TILEMAP: \n[\n";
    for (int y = 0; y < tilemap.height; y++) {
        std::cout << "[";
        for (int x = 0; x < tilemap.width; x++) {
            std::cout << tilemap.tile_ids[x + tilemap.width * y] << ", ";
        }
        std::cout << "]\n";
    }
    std::cout << "]\n";
}

void print_tileset(TileSet &tileset) {
    std::cout << "TILESET: \n[\n";
    for (int y = 0; y < tileset.dimensions.y; y++) {
        std::cout << "[";
        for (int x = 0; x < tileset.dimensions.x; x++) {
            std::cout << (int)tileset.collisions[x + tileset.dimensions.x * y] << ", ";
        }
        std::cout << "]\n";
    }
    std::cout << "]\n";
}
#endif

void generate_collision(TileMap &tilemap, TileSet &tileset) {
    auto &tileset_collision_info = tileset.collisions;
    for (int y = 1; y < tilemap.height - 1; y++) {
        for (int x = 1; x < tilemap.width - 1; x++) {
            if (tileset_collision_info[tilemap.tile_ids[GET_2D_POS(x, y, tilemap.width)] - 1]) {
                createWall(vec2(x * TILE_SIZE_PX, y * TILE_SIZE_PX));
            }
        }
    }
}
// tiles -> wall entites
void init_tile_walls() {
    TileMap &tilemap = registry.tilemaps.components[0];
    TileSet &tileset = registry.tilesets.components[0];

    int walls_created = 0;
    int floor_tile_id = 1; // guessing -> confirmed from output
    for (unsigned int y = 0; y < tilemap.height; y++) {
        for (unsigned int x = 0; x < tilemap.width; x++) {
            int tile_id = tilemap.tile_ids[x + y * tilemap.width];

            // skip empty/floor tiles, skip outer border tiles
            bool is_border = (y == 0 || y == tilemap.height - 1 || x == 0 || x == tilemap.width - 1);
            bool is_non_empty = (tile_id != floor_tile_id);

            if (is_non_empty && !is_border) {
                vec2 position = vec2(x * TILE_SIZE_PX + TILE_SIZE_PX / 2.f, y * TILE_SIZE_PX + TILE_SIZE_PX / 2.f);
                vec2 scale = vec2(TILE_SIZE_PX, TILE_SIZE_PX);
                createWall(position, scale);
                walls_created++;
            }
        }
    }
}
void init_tiles() {
    Entity entity = Entity("Tiles");

    TileMap tilemap = parse_tilemap(TILEMAP_JSON);
    TileSet tileset = parse_tileset(TILESET_JSON);

    registry.tilemaps.insert(entity, tilemap);
    registry.tilesets.insert(entity, tileset);

    registry.renderRequests.insert(
        entity, {(TEXTURE_ASSET_ID)tilemap.tileset_id, EFFECT_ASSET_ID::TILEMAP, GEOMETRY_BUFFER_ID::SPRITE});

#ifdef DEBUG_INIT_TILES
    print_tilemap(tilemap);
    print_tileset(tileset);
#endif
}

#pragma region Actors
// =================================================================================
// Actors
// =================================================================================

// Player creation function
Entity createPlayer(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Player");

    PlayerConfig config;
    registry.players.emplace(entity);

    registry.keyboardInput.emplace(entity);
    registry.mouseInput.emplace(entity);

    Wallet &w = registry.wallet.emplace(entity);
    w.crackers = config.crackers;

    Health &h = registry.health.emplace(entity);
    h.hp = config.hp;
    h.max_hp = config.max_hp;

    FireControl &fc = registry.fireControl.emplace(entity);
    fc.cooldown_ms = config.fire_rate_cd_ms;
    fc.ammo = STARTING_AMMO;

    SpeedControl &sc = registry.speedControl.emplace(entity);
    sc.move_speed = config.move_speed;
    sc.turn_speed = config.turn_speed;

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
    registry.meshPtrs.emplace(entity, &mesh);

    Motion &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(50.0f, 50.0f);

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::DEBUG_LINE});

    registry.colors.emplace(entity, config.colour);

    createTurret(renderer, position);
    return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, ENEMY_TYPE type) {
    auto entity = Entity("Enemy");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
    registry.meshPtrs.emplace(entity, &mesh);

    auto &config = ENEMY_CONFIGS[(int)type];

    auto &enemy = registry.enemies.emplace(entity);
    enemy.detection_radius = config.detection_radius;

    auto &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(50.0f, 50.0f);

    registry.health.emplace(entity);

    auto &fire = registry.fireControl.emplace(entity);
    fire.cooldown_ms = ATTACK_CONFIGS[(int)config.attack_pattern].cooldown_ms;
    fire.attack_pattern = config.attack_pattern;

    auto &speed = registry.speedControl.emplace(entity);
    speed.move_speed = config.move_speed;
    speed.turn_speed = config.turn_speed;

    registry.dropsCrackers.emplace(entity);

    if (type == ENEMY_TYPE::PATROLLER) {
        registry.patrolling.emplace(entity);
    }

    registry.colors.emplace(entity, vec3(1.0f, 0.2f, 0.2f));
    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::DEBUG_LINE});

    return entity;
}

Entity createCamera(vec2 position) {
    auto entity = Entity("Camera");

    registry.camera.emplace(entity);
    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    return entity;
}

Entity createTurret(RenderSystem *renderer, vec2 position) {
    Entity entity = Entity("Turret");

    Turret &t = registry.turrets.emplace(entity);
    t.barrel_length_px = 30.f;

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    Motion &m = registry.motions.emplace(entity);
    m.position = position;
    m.velocity = {0.f, 0.f};
    m.angle = 0.f;
    m.scale = {70.f, 18.f};

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_BLACK, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE, 1});

    // registry.colors.emplace(entity, vec3(0.f, 0.f, 0.f));

    return entity;
}

/*Entity createEnemy(RenderSystem* renderer, vec2 position, vec2 patrol_a, vec2 patrol_b, float speed_px_s)
{
    auto entity = Entity("Enemy");
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
}*/

#pragma endregion Actors

#pragma region Pickups
// =================================================================================
// Pickups
// =================================================================================

Entity createPowerUp(RenderSystem *renderer, vec2 position, POWER_UP_TYPE type) {
    auto entity = Entity("Power Up");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EGG);
    registry.meshPtrs.emplace(entity, &mesh);

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {40.f, 40.f};

    auto &config = POWER_UP_CONFIGS[(int)type];

    registry.colors.emplace(entity, config.colour);
    registry.renderRequests.insert(entity,
                                   {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::EGG});

    registry.pickUp.emplace(entity);
    registry.powerUp.emplace(entity, type);

    return entity;
}

Entity createBubbler(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Power Up");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    registry.bubblers.emplace(entity);

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {TILE_SIZE_PX, TILE_SIZE_PX};

    // ParticleEmitter &pe = registry.particleEmitters.emplace(entity);
    // pe.life_ms = 1;
    // pe.particle_life_ms = 400;
    // pe.max_particles = 10;
    // pe.particle_speed = 75;
    // pe.spawn_interval_ms = 100;
    // pe.position = motion.position;

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_BUBBLER, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

#pragma endregion Pickups

#pragma region Distractions
// =================================================================================
// Distractions
// =================================================================================

Entity createBell(RenderSystem *renderer, vec2 position, float interact_radius_px) {
    auto entity = Entity("Bell");
    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EGG);
    registry.meshPtrs.emplace(entity, &mesh);

    auto &bell = registry.bells.emplace(entity);
    bell.interact_radius_px = interact_radius_px;
    bell.cooldown_ms = 800.f;
    bell.cooldown_timer = 0.f;

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {30.f, 30.f};

    registry.colors.emplace(entity, vec3(0.1f, 0.9f, 0.1f));
    registry.renderRequests.insert(entity,
                                   {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::EGG});

    return entity;
}

#pragma endregion Distractions

#pragma region Hazards
// =================================================================================
// Hazards
// =================================================================================

Entity createProjectile(vec2 position, float angle, ATTACK_PATTERN attack_pattern, Entity owner) {
    auto entity = Entity("Projectile");

    AttackConfig attack_config = ATTACK_CONFIGS[(int)attack_pattern];

    // Apply DamageMultiplier power up
    for (Entity &dm_entity : registry.damageMultiplier.entities) {
        if (registry.target.has(dm_entity)) {
            Target &t = registry.target.get(dm_entity);
            if (t.target == owner) {
                DamageMultiplier &dm = registry.damageMultiplier.get(dm_entity);
                attack_config.damage *= dm.multiplier;
            }
        }
    }

    Bullet &bullet = registry.bullet.insert(entity, {attack_config.damage, owner});
    registry.expiration.emplace(entity, attack_config.lifespan_ms);

    Motion &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(attack_config.projectile_speed, 0.f);
    motion.position = position;
    motion.angle = angle;
    motion.scale = vec2(PROJECTILE_SIZE_PX);

    Bouncy &bouncy = registry.bouncy.emplace(entity);

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_BUBBLE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    ParticleEmitter &pe = registry.particleEmitters.emplace(entity);
    pe.offset = {-20, 0};
    pe.max_particles = 500;
    pe.texture = TEXTURE_ASSET_ID::TEXTURE_BUBBLE;
    pe.particle_speed = 80;
    pe.spawn_interval_ms = 40;
    pe.particle_life_ms = 200;

    return entity;
}

Entity createMine(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Mine");

    auto &config = HAZARD_CONFIGS[(int)HAZARD_TYPE::MINE];
    registry.mine.emplace(entity);
    registry.hazard.emplace(entity, HAZARD_TYPE::MINE);

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EGG);
    registry.meshPtrs.emplace(entity, &mesh);

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {40.f, 40.f};

    registry.colors.emplace(entity, config.colour);
    registry.renderRequests.insert(entity,
                                   {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::EGG});

    return entity;
}

Entity createAnemone(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Anemone");

    auto &config = HAZARD_CONFIGS[(int)HAZARD_TYPE::ANEMONE];
    registry.anemone.emplace(entity);
    registry.hazard.emplace(entity, HAZARD_TYPE::ANEMONE);

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EGG);
    registry.meshPtrs.emplace(entity, &mesh);

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {40.f, 40.f};

    registry.colors.emplace(entity, config.colour);
    registry.renderRequests.insert(entity,
                                   {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::EGG});

    return entity;
}

#pragma endregion Hazards

#pragma region StaticObjects

Entity createWall(const vec2 &position, const vec2 &scale) {
    auto entity = Entity("Wall");

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = vec2(0.f, 0.f);
    motion.angle = 0.f;

    registry.wall.emplace(entity);

    return entity;
}
#pragma endregion StaticObjects

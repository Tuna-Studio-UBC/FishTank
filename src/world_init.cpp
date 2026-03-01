#include "world_init.hpp"
#include "configs/attack_pattern.cpp"
#include "configs/enemy_config.cpp"
#include "configs/player_config.cpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <map>
#include <set>

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

    // Replace the debug-line mesh pointer with the sprite mesh so original_size matches the texture
    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    Motion &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(80.0f, 80.0f);

    MeshCollider fishMesh;
    fishMesh.vertices = {
        {-42.f, 0.f},   // tail tip (left)
        {-20.f, -25.f}, // tail top
        {10.f, -18.f},  // mid top
        {40.f, -12.f},  // upper body
        {64.f, -4.f},   // nose top
        {64.f, 4.f},    // nose bottom
        {40.f, 12.f},   // lower body
        {10.f, 18.f},   // mid bottom
        {-20.f, 25.f}   // tail bottom
    };

    // compute authored bbox and set authoring_px from it
    {
        vec2 minV = fishMesh.vertices[0];
        vec2 maxV = fishMesh.vertices[0];
        for (const vec2 &v : fishMesh.vertices) {
            minV.x = std::min(minV.x, v.x);
            minV.y = std::min(minV.y, v.y);
            maxV.x = std::max(maxV.x, v.x);
            maxV.y = std::max(maxV.y, v.y);
        }
        float authored_width = maxV.x - minV.x;
        float authored_height = maxV.y - minV.y;
        fishMesh.authoring_px =
            std::max(1.0f, std::max(authored_width, authored_height)); // fallback to 1 if degenerate

        // center the collider geometry around its centroid so physics SAT uses same origin as renderer.
        vec2 centroid = (minV + maxV) * 0.5f;
        for (vec2 &v : fishMesh.vertices)
            v -= centroid;
    }

    registry.meshColliders.emplace(entity, std::move(fishMesh));

    // keep fishMesh.authoring_px computed from authored vertices; do not overwrite it here.

    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_FISH_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::ACTORS});

    Animation &an = registry.animations.emplace(entity);
    an.columns = 9;
    an.frame_count = 9;
    an.frame_time = 60;

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

    auto &health = registry.health.emplace(entity);
    health = config.health;

    auto &fire = registry.fireControl.emplace(entity);
    fire.cooldown_ms = ATTACK_CONFIGS[(int)config.attack_pattern].cooldown_ms;
    fire.attack_pattern = config.attack_pattern;

    auto &speed = registry.speedControl.emplace(entity);
    speed.move_speed = config.move_speed;
    speed.turn_speed = config.turn_speed;

    registry.senses.emplace(entity);
    AI &ai = registry.ais.emplace(entity);
    ai.tree_file = "enemy_tree.txt";
    ai.tree = load_tree(ai.tree_file);
    ai.current_state = AIAction::IDLE;

    if (type != ENEMY_TYPE::URCHIN) {
        registry.pursuit.emplace(entity);
        registry.distracted.emplace(entity);
    }

    registry.dropsCrackers.emplace(entity);

    if (type == ENEMY_TYPE::URCHIN) {
        registry.colors.emplace(entity, vec3(1.0f, 0.2f, 0.2f));
        registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_URCHIN_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                                GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::ACTORS});

        Animation &an = registry.animations.emplace(entity);
        an.frame_count = 4;
        an.columns = 4;
        an.frame_time = 500;
    } else if (type == ENEMY_TYPE::HOMING) {
        registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_ISOPOD_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                                GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::ACTORS});

        Animation &an = registry.animations.emplace(entity);
        an.frame_count = 8;
        an.columns = 8;
        an.frame_time = 125;
        motion.scale = vec2(40.0f, 25.0f);
    } else {
        registry.colors.emplace(entity, vec3(1.0f, 0.2f, 0.2f));
        registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_CRAB_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                                GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::ACTORS});

        Animation &an = registry.animations.emplace(entity);
        an.frame_count = 8;
        an.columns = 8;
        an.frame_time = 250;
    }

    return entity;
}

Entity createLeader(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Enemy");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
    registry.meshPtrs.emplace(entity, &mesh);

    registry.beziers.emplace(entity);

    auto &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(25.0f, 25.0f);

    /* auto &speed = registry.speedControl.emplace(entity);
    speed.move_speed = config.move_speed;
    speed.turn_speed = config.turn_speed;*/

    registry.senses.emplace(entity);
    AI &ai = registry.ais.emplace(entity);
    ai.tree_file = "leader_tree.txt";
    ai.tree = load_tree(ai.tree_file);
    ai.current_state = AIAction::IDLE;

    registry.colors.emplace(entity, vec3(0.2f, 0.2f, 1.0f));
    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::DEBUG_LINE});

    return entity;
}

Entity createFollower(RenderSystem *renderer, vec2 position, Entity leader) {
    auto entity = Entity("Enemy");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
    registry.meshPtrs.emplace(entity, &mesh);

    Boid &boid = registry.boids.emplace(entity);
    boid.leader = leader;

    auto &motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.0f, 0.0f);
    motion.position = position;
    motion.angle = 0.f;
    motion.scale = vec2(25.0f, 25.0f);

    /* auto &speed = registry.speedControl.emplace(entity);
    speed.move_speed = config.move_speed;
    speed.turn_speed = config.turn_speed;*/

    registry.colors.emplace(entity, vec3(0.2f, 0.2f, 1.0f));
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
    t.barrel_length_px = 10.f;

    Motion &m = registry.motions.emplace(entity);
    m.position = position;
    m.velocity = {0.f, 0.f};
    m.angle = 0.f;
    m.scale = {48.f, 24.f};

    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_TURRET_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::TURRETS});

    Animation &an = registry.animations.emplace(entity);
    an.columns = 5;
    an.frame_count = 5;
    an.frame_time = 240;

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
          GEOMETRY_BUFFER_ID::DEBUG_LINE,
          RENDER_LAYER::ACTORS });

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

    if (config.type == POWER_UP_TYPE::HEAL) {
        registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_HEALTH_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                                GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::INTERACTABLES});
        Animation &an = registry.animations.emplace(entity);
        an.columns = 4;
        an.frame_count = 4;
        an.frame_time = 250;
    } else {
        registry.colors.emplace(entity, config.colour);
        registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG,
                                                GEOMETRY_BUFFER_ID::EGG, RENDER_LAYER::INTERACTABLES});
    }

    registry.pickUp.emplace(entity);
    registry.powerUp.emplace(entity, type);

    return entity;
}

Entity createBubbler(RenderSystem *renderer, vec2 position) {
    auto entity = Entity("Bubbler");

    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    registry.bubblers.emplace(entity);

    auto &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.velocity = {0.f, 0.f};
    motion.angle = 0.f;
    motion.scale = {TILE_SIZE_PX, TILE_SIZE_PX};

    ParticleEmitter &pe = registry.particleEmitters.emplace(entity);
    pe.particle_life_ms = 400;
    pe.max_particles = 10;
    pe.particle_speed = 75;
    pe.spawn_interval_ms = 100;
    pe.particle_scale = 10;
    pe.texture = TEXTURE_ASSET_ID::TEXTURE_BUBBLE;

    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_BUBBLER, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::INTERACTABLES});

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
    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG,
                                            GEOMETRY_BUFFER_ID::EGG, RENDER_LAYER::INTERACTABLES});

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

    registry.tints.emplace(entity, attack_config.projectile_tint);
    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_BUBBLE, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::PROJECTILES});

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

    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_MINE_SHEET, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::INTERACTABLES});
    Animation &an = registry.animations.emplace(entity);
    an.columns = 3;
    an.frame_count = 3;
    an.frame_time = 500;

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
    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG,
                                            GEOMETRY_BUFFER_ID::EGG, RENDER_LAYER::INTERACTABLES});

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

Entity createTutorialTrigger(const vec2 position, const vec2 size, const Tutorial id) {
    Entity entity = Entity("Tutorial Trigger " + id);

    registry.tutorialTriggers.emplace(entity, id);

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = size;

// #define DRAW_TUTORIAL_TRIGGERS
#ifdef DRAW_TUTORIAL_TRIGGERS
    registry.colors.emplace(entity, 1.f, 0.f, 0.f);
    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG,
                                            GEOMETRY_BUFFER_ID::DEBUG_LINE, RENDER_LAYER::INTERACTABLES});
#endif

    return entity;
}

#pragma endregion StaticObjects

#pragma region Effects

Entity createEffectSmoke(const vec2 position) {
    Entity e = Entity("Particle Emitter (Smoke)");
    ParticleEmitter &pe = registry.particleEmitters.emplace(e);
    pe.life_ms = 500;
    pe.particle_life_ms = 500;
    pe.max_particles = 20;
    pe.particle_speed = 60;
    pe.spawn_interval_ms = 30;
    pe.standalone = true;
    pe.texture = TEXTURE_ASSET_ID::TEXTURE_SMOKE;
    pe.particle_scale = 50;
    Motion &pm = registry.motions.emplace(e);
    pm.position = position;
    return e;
}

Entity createEffectBubbles(const vec2 position) {
    Entity e = Entity("Particle Emitter (Bubble)");
    ParticleEmitter &pe = registry.particleEmitters.emplace(e);
    pe.life_ms = 500;
    pe.particle_life_ms = 500;
    pe.max_particles = 1000;
    pe.particle_speed = 120;
    pe.spawn_interval_ms = 50;
    pe.standalone = true;
    pe.texture = TEXTURE_ASSET_ID::TEXTURE_BUBBLE;
    pe.particle_scale = 10;
    Motion &pm = registry.motions.emplace(e);
    pm.position = position;
    return e;
}

#pragma endregion Effects

#pragma region ScreenOverlays

Entity createText(std::string text, vec2 position, float scale, const glm::vec3 &color, const glm::mat4 &trans) {
    auto entity = Entity("Text");

    Text &t = registry.text.emplace(entity);
    t.text = text;
    t.position = position;
    t.scale = scale;
    t.color = color;
    t.trans = trans;

    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::FONT,
                                            GEOMETRY_BUFFER_ID::EGG, RENDER_LAYER::OVERLAYS});

    return entity;
}

#pragma endregion ScreenOverlays

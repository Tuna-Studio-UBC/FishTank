#include "level_system.hpp"

int LevelSystem::get_current_wave() const {
    Persistence &persist = registry.persistences.components[0];
    return persist.wave;
}

int LevelSystem::get_total_waves_for_current_level() const {
    Persistence &persist = registry.persistences.components[0];
    int level = persist.level;
    if (level < 0 || level >= (int)LEVEL_CONFIGS.size()) {
        return 0;
    }

    return LEVEL_CONFIGS[level].last_wave + 1;
}

bool LevelSystem::is_in_tutorial() const {
    return !registry.progresses.components.empty() && registry.progresses.components[0].is_tutorial;
}

void LevelSystem::step(const float &elapsed_ms) {
    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    if (progress.trigger_restart) {
        restart_game();
        return;
    }

    if (progress.trigger_load) {
        load_game();
        return;
    }

    if (progress.is_game_over || progress.is_paused) {
        return;
    }

    if (should_advance()) {
        assert(registry.persistences.size() > 0 && "Persistence component not found");
        Persistence &persist = registry.persistences.components[0];
        spdlog::info("Wave {} complete, starting next wave.", persist.wave + 1);
        if (progress.is_tutorial) {
            trigger_load_next_level();
        } else {
            if (persist.wave == LEVEL_CONFIGS[persist.level].last_wave && persist.level < LEVEL_CONFIGS.size() - 1) {
                trigger_load_next_level();
            } else if (persist.wave < (int)WAVE_CONFIGS.size() - 1 &&
                       persist.wave < LEVEL_CONFIGS[persist.level].last_wave) {
                trigger_load_next_wave();
            } else {
                trigger_win(progress);
            }
        }
    }

    for (Entity e : registry.spawnRequests.entities) {
        SpawnRequest &req = registry.spawnRequests.get(e);
        req.timer_ms -= elapsed_ms;
        if (req.timer_ms <= 0) {
            Entity entity = createEnemy(renderer, req.position, req.type);
            createEffectSmoke(req.position);
            if (req.type == ENEMY_TYPE::MOTHER) {
                Bezier &b = registry.beziers.get(entity);
                Motion &m = registry.motions.get(entity);
                std::vector<BezierPath> &loop = registry.tilemaps.get(tilemap_system->get_current_map()).patrol_loop;
                for (int i = 0; i < loop.size(); i++) {
                    vec2 p0 = loop[i].p0;
                    if (m.position.x == p0.x && m.position.y == p0.y) {
                        b.current_segment = i;
                        break;
                    }
                }
                for (int i = 1; i < 7; i++) {
                    createFollower(renderer, tilemap_system->get_random_position(SpawnTarget::ENEMY), entity);
                }
            }

            registry.spawnRequests.remove(e);
        }
    }

    handle_timer(elapsed_ms);
}

void LevelSystem::handle_timer(const float &elapsed_ms) {
    advance_timer = max(0.f, advance_timer - elapsed_ms);
    time_since_last_wave = min(time_since_last_wave + elapsed_ms, 300000.f); // Capped to not have floating point issues
    time_since_level_start = min(time_since_level_start + elapsed_ms, 3000000.f);

    if (advance_timer == 0.f) {
        if (triggered_load_level) {
            load_next_level();
        } else if (triggered_load_wave) {
            load_next_wave();
        }
    }
}

void LevelSystem::trigger_win(Progress &progress) {
    progress.is_game_over = true;
    progress.has_beat_level = true;

    // TODO: Also change this to a GUI overlay
    Entity e = Entity("Overlay");
    registry.overlays.emplace(e);
    auto &motion = registry.motions.emplace(e);
    motion.position = registry.motions.get(registry.cameras.entities[0]).position;
    motion.scale = vec2(TILE_SIZE_PX * 10, TILE_SIZE_PX * 10);

    registry.renderRequests.insert(e, {TEXTURE_ASSET_ID::TEXTURE_WIN_OVERLAY, EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE, RENDER_LAYER::OVERLAYS});

    // Reset persistence
    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    persist.reset();
    persist.level = 0;
    persist.wave = -1;
}

void LevelSystem::load_wave(int wave) {
    enemies_spawned_last_wave = 0;
    time_since_last_wave = 0.f;
    if (wave >= WAVE_CONFIGS.size())
        return;
    auto config = WAVE_CONFIGS[wave];
    vec2 pos;

    if (registry.players.entities.size() == 0) {
        // Create player entity
        pos = tilemap_system->get_random_position(SpawnTarget::PLAYER);
        Entity player = createPlayer(renderer, pos);

        // Create camera
        camera_system->init(tilemap_system->get_current_map());
    }

    for (auto enemy : config.enemies) {
        if (enemy == ENEMY_TYPE::MOTHER) {
            if (registry.tilemaps.has(tilemap_system->get_current_map())) {
                int idx = 0;
                for (SpawnRequest sr : registry.spawnRequests.components) {
                    idx += sr.type == ENEMY_TYPE::MOTHER ? 1 : 0;
                }
                spdlog::debug("spawning mother at the {} curve", idx);
                Entity sp_e = Entity();
                BezierPath &path = registry.tilemaps.get(tilemap_system->get_current_map()).patrol_loop[idx];
                pos = path.p0;
            }
        } else {
            pos = tilemap_system->get_random_position(SpawnTarget::ENEMY);
        }

        createEffectSpawn(pos);
        Entity e = Entity();
        registry.spawnRequests.insert(e, {pos, 2000.f, enemy});
        enemies_spawned_last_wave++;
    }

    for (auto boss : config.bosses) {
        // TODO: Replace w/ just BOSS
        pos = tilemap_system->get_random_position(SpawnTarget::BOSS | SpawnTarget::ENEMY);
        createBoss(renderer, pos, boss);
        enemies_spawned_last_wave++;
    }
}

void LevelSystem::load_level_components() {
    tilemap_system->init_spawn_points();

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    auto config = LEVEL_CONFIGS[persist.level];

    vec2 pos;
    for (auto item : config.hazards) {
        pos = tilemap_system->get_random_position(SpawnTarget::HAZARD);
        if (item == HAZARD_TYPE::MINE) {
            createMine(renderer, pos);
        } else if (item == HAZARD_TYPE::ANEMONE) {
            createAnemone(renderer, pos, false);
        }
    }

    for (ivec2 bubbler : config.bubblers) {
        pos = bubbler * TILE_WIDTH_PX + (TILE_WIDTH_PX / 2);
        createBubbler(renderer, pos);
    }

    for (auto power_up : config.powerups) {
        pos = tilemap_system->get_random_position(SpawnTarget::POWERUP);
        createPowerUp(renderer, pos, power_up);
    }
}

void LevelSystem::apply_upgrades(Entity &player, Upgrades &upgrades) {
    if (registry.fireControls.has(player)) {
        FireControl &fc = registry.fireControls.get(player);
        int bonus = upgrades.upgrade_instances[MAX_AMMO].config.bonus[upgrades.upgrade_instances[MAX_AMMO].level];
        fc.max_ammo = STARTING_AMMO + bonus;
        fc.ammo = STARTING_AMMO + bonus;
        bonus = upgrades.upgrade_instances[ATTACK].config.bonus[upgrades.upgrade_instances[ATTACK].level];
        fc.damage_bonus = bonus;
    }
    if (registry.healths.has(player)) {
        Health &health = registry.healths.get(player);
        int bonus = upgrades.upgrade_instances[MAX_HP].config.bonus[upgrades.upgrade_instances[MAX_HP].level];
        health.max_hp = 100 + bonus;
        health.hp = 100 + bonus;
    }
}

void LevelSystem::load_level(int level, bool should_persist) {
    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress progress = registry.progresses.components[0];
    const bool was_tutorial = progress.is_tutorial;
    progress.is_tutorial = false;
    progress.is_tutorial_step_called = false;
    progress.has_beat_level = false;
    progress.is_game_over = false;

    if (level >= LEVEL_CONFIGS.size()) {
        world_system->close_window();
        return;
    }

    progress.is_upgrade_menu_called = LEVEL_CONFIGS[level].upgrade_screen_before;
    progress.is_paused = progress.is_upgrade_menu_called;

    bool player_exists = registry.players.size() > 0;

    // Persist upgrades
    if (player_exists && !was_tutorial && should_persist) {
        Entity player = registry.players.entities[0];
        Persistence &persist = registry.persistences.components[0];
        persist.wallet = registry.wallets.get(player);
        persist.upgrades = registry.upgrades.get(player);
        persist.score = registry.scoring.get(player);
    }

    particle_system.clear_all_particles();
    registry.clear_all_components();

    registry.progresses.insert(Entity("Progress"), progress);

    tilemap_system->init();
    tilemap_system->load_map(LEVEL_CONFIGS[level].map_type);
    load_level_components();

    // Create player entity
    vec2 pos = tilemap_system->get_random_position(SpawnTarget::PLAYER);
    Entity player = createPlayer(renderer, pos);

    // Create camera
    camera_system->init(tilemap_system->get_current_map());

    if (player_exists && !was_tutorial && should_persist) {
        Entity player = registry.players.entities[0];
        Persistence &persist = registry.persistences.components[0];
        registry.wallets.get(player) = persist.wallet;
        registry.upgrades.get(player) = persist.upgrades;
        registry.scoring.get(player) = persist.score;
        apply_upgrades(player, persist.upgrades);
    }

    time_since_last_wave = 0.f;
}

void LevelSystem::load_tutorial() {
    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    persist.reset();
    persist.level = -1;
    persist.wave = -1;

    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    progress.is_tutorial = true;
    progress.is_tutorial_step_called = false;
    progress.tutorial_step_id = Tutorial::MOVEMENT;
    progress.tutorial_steps = TUTORIAL_STEPS;

    // Set up world level
    tilemap_system->init();
    tilemap_system->load_map(MapType::TUTORIAL_MAP);

    glm::vec3 font_color = glm::vec3(0.f, 1.f, 0.f);
    createText("Tutorial", vec2(WINDOW_WIDTH_PX / 2 - 50, WINDOW_HEIGHT_PX - 48), 1.f, font_color, glm::mat4(1.f));

    Entity player = createPlayer(renderer, {250.f, 1750.f});
    Motion &player_motion = registry.motions.get(player);
    player_motion.angle = -M_PI / 2;
    FireControl &player_fc = registry.fireControls.get(player);
    player_fc.ammo = 10;

    createLabelText("W - SWIM", player_motion.position, vec3(1.f), OFFSET::TOP);
    createLabelText("A - LEFT", player_motion.position, vec3(1.f), OFFSET::LEFT);
    createLabelText("D - RIGHT", player_motion.position, vec3(1.f), OFFSET::RIGHT);
    createLabelText("S - REVERSE", player_motion.position, vec3(1.f), OFFSET::BOTTOM);

    createEnemy(renderer, {150.f, 150.f}, ENEMY_TYPE::DASHER);
    createEnemy(renderer, {850.f, 150.f}, ENEMY_TYPE::DASHER);
    createEnemy(renderer, {500.f, 150.f}, ENEMY_TYPE::GUARD);

    // Set enemy health to something low for tutorial
    for (Entity e : registry.enemies.entities) {
        if (registry.fireControls.has(e)) {
            registry.fireControls.get(e).cooldown_ms = 5000.f;
        }
        registry.detections.insert(e, {400.f});
    }

    Entity target = createBubbler(renderer, {750.f, 875.f});
    registry.bubblers.get(target).ammo_given = 30;
    createLabelText("Pick up more ammo!", target, vec3(0.95f, 0.65f, 0.38f), OFFSET::TOP);
    createLabelText("Come back later for more", target, vec3(0.95f, 0.65f, 0.38f), OFFSET::BOTTOM);
    createBubbler(renderer, {425.f, 400.f});

    target = createMine(renderer, {450.f, 1100.f});
    registry.explosives.get(target).damage = 25.f;
    createLabelText("Mine", target, vec3(1.f, 0.f, 0.f), OFFSET::BOTTOM);
    target = createAnemone(renderer, {350.f, 1000.f}, true);
    createLabelText("Anemone", target, vec3(1.f, 0.f, 0.f), OFFSET::BOTTOM);
    target = createPowerUp(renderer, {900.f, 1300.f}, POWER_UP_TYPE::HEAL);
    createLabelText("Health", target, vec3(0.f, 1.f, 0.f), OFFSET::TOP);
    target = createPowerUp(renderer, {750.f, 1300.f}, POWER_UP_TYPE::DAMAGE_MULTIPLIER);
    createLabelText("More Damage", target, vec3(0.f, 1.f, 0.f), OFFSET::TOP);
    target = createPowerUp(renderer, {600.f, 1300.f}, POWER_UP_TYPE::SPEED_MULTIPLIER);
    createLabelText("More Speed", target, vec3(0.f, 1.f, 0.f), OFFSET::TOP);

    createTutorialTrigger(player_motion.position, vec2(50.f, 50.f), Tutorial::MOVEMENT);
    Entity label = createLabelText("INTERACT", {250.f, 1200.f}, vec3(1.f), OFFSET::TOP);
    registry.texts.get(label).scale = 1.5f;
    label = createLabelText("COMBAT", {750.f, 775.f}, vec3(1.f), OFFSET::TOP);
    registry.texts.get(label).scale = 1.5f;

    createLabelText("1/2/3 - Switch Weapon", {825.f, 350.f}, vec3(1.f), OFFSET::TOP);
    createLabelText("Left - Attack 1", {825.f, 400.f}, vec3(1.f), OFFSET::TOP);
    createLabelText("Right - Attack 2", {825.f, 450.f}, vec3(1.f), OFFSET::TOP);
    createLabelText("F - Parry", {125.f, 400.f}, vec3(1.f), OFFSET::TOP);
    createLabelText("SHIFT - Dash", {125.f, 450.f}, vec3(1.f), OFFSET::TOP);
    createLabelText("Point and Click!", {825.f, 550.f}, vec3(1.f), OFFSET::TOP);

    camera_system->init(tilemap_system->get_current_map()); // Must be done last, requires player to exist
}

void LevelSystem::load_game() {
    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    progress.trigger_load = false;

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];

    particle_system.clear_all_particles();
    registry.clear_all_components();
    registry.progresses.emplace(Entity("Progress"));

    if (persist.level == -1) {
        load_tutorial();
    } else {
        load_level(persist.level, false);

        persist.wave = (persist.level > 0) ? LEVEL_CONFIGS[persist.level - 1].last_wave : -1;
        trigger_load_next_wave();

        assert(registry.players.size() > 0 && "Player component not found");
        Entity player = registry.players.entities[0];
        registry.wallets.get(player) = persist.wallet;
        registry.upgrades.get(player) = persist.upgrades;
        registry.scoring.get(player) = persist.score;
        apply_upgrades(player, persist.upgrades);
    }
}

void LevelSystem::restart_game() {
    bool is_tutorial = registry.progresses.components[0].is_tutorial;

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];

    // Persist any stats if not first level or tutorial
    if (!is_tutorial && persist.level > 0) {
        assert(registry.players.size() > 0 && "Player component not found");
        Entity player = registry.players.entities[0];
        persist.wallet = registry.wallets.get(player);
        persist.upgrades = registry.upgrades.get(player);
        persist.score = registry.scoring.get(player);
    }

    particle_system.clear_all_particles();
    registry.clear_all_components();
    registry.progresses.emplace(Entity("Progress"));

    if (is_tutorial) {
        load_tutorial();
    } else {
        // Load level, but don't persist since registry was cleared
        load_level(persist.level, false);

        // Load from last wave
        persist.wave = (persist.level > 0) ? LEVEL_CONFIGS[persist.level - 1].last_wave : -1;
        trigger_load_next_wave();

        // Reset any stats
        if (persist.level > 0) {
            assert(registry.players.size() > 0 && "Player component not found");
            Entity player = registry.players.entities[0];
            registry.wallets.get(player) = persist.wallet;
            registry.upgrades.get(player) = persist.upgrades;
            registry.scoring.get(player) = persist.score;
            apply_upgrades(player, persist.upgrades);
        }
    }
}

bool LevelSystem::should_advance() {
    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    if (progress.trigger_skip_wave) {
        progress.trigger_skip_wave = false;
        return true;
    }

    int num_enemies = registry.enemies.entities.size();
    int num_spawn_requests = registry.spawnRequests.entities.size();

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    if (persist.level == -1)
        return advance_timer == 0.f && num_enemies == 0 && num_spawn_requests == 0;

    const Level &level_ref = LEVEL_CONFIGS[persist.level];
    if (persist.wave == level_ref.last_wave) {
        return advance_timer == 0.f && num_enemies == 0 && num_spawn_requests == 0;
    } else if ((num_enemies + num_spawn_requests) / (float)enemies_spawned_last_wave <= level_ref.spawn_threshold) {
        return advance_timer == 0.f;
    } else if (time_since_last_wave > level_ref.time_threshold) {
        time_since_last_wave = 0.f;
        return true;
    }
    return false;
}

void LevelSystem::load_next_level() {
    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    progress.is_transitioning_level = false;

    triggered_load_level = false;
    time_since_level_start = 0.f;

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    persist.level++;

    load_level(persist.level);
    FireControl &fc = registry.fireControls.get(registry.players.entities[0]);
    fc.ammo = fc.max_ammo;
    trigger_load_next_wave();
}

void LevelSystem::load_next_wave() {
    triggered_load_wave = false;

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    persist.wave++;

    load_wave(persist.wave);
}

void LevelSystem::trigger_load_next_level() {
    triggered_load_level = true;
    advance_timer = 2000.f;

    assert(registry.progresses.size() > 0 && "Progress component not found");
    Progress &progress = registry.progresses.components[0];
    progress.is_transitioning_level = true;
    if (progress.force_skip_tutorial) {
        progress.force_skip_tutorial = false;
        advance_timer = 0.f;
    }

    assert(registry.persistences.size() > 0 && "Persistence component not found");
    Persistence &persist = registry.persistences.components[0];
    if (persist.level == -1) {
        createCompletionText("TUTORIAL COMPLETE!!");
    } else {
        createCompletionText("LEVEL " + std::to_string(persist.level + 1) + " COMPLETE!!");
        handle_time_bonus();
    }
}

void LevelSystem::handle_time_bonus() { 
    Scoring& scoring = registry.scoring.components[0];
    Persistence &persist = registry.persistences.components[0];
    Wallet &wallet = registry.wallets.components[0];
    float relative_time = time_since_level_start - LEVEL_CONFIGS[persist.level].time_bonus_threshold;
    int crackers_earned = 5;
    if (relative_time > 0.f) {
        crackers_earned = max(0, 4 - ((int)relative_time / 5000));
    }
    scoring.crackers_earned += crackers_earned;
    wallet.crackers += crackers_earned;
    createTimeBonusText(crackers_earned * 1000);
}

void LevelSystem::trigger_load_next_wave() {
    triggered_load_wave = true;
    advance_timer = 50.f;
}

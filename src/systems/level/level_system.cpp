#include "level_system.hpp"

void LevelSystem::step(const float &elapsed_ms) {
    Progress &progress = registry.progresses.components[0];
    if (progress.trigger_restart) {
        restart_game();
        return;
    }

    if (progress.is_game_over || progress.is_paused) {
        return;
    }

    if (should_advance()) {
        spdlog::info("Wave {} complete, starting next wave.", wave + 1);

        if (progress.is_tutorial) {
            trigger_load_next_level();
        } else {
            if (wave == LEVEL_CONFIGS[level].last_wave && level < LEVEL_CONFIGS.size() - 1) {
                trigger_load_next_level();
            } else if (wave < WAVE_CONFIGS.size() - 1 && wave < LEVEL_CONFIGS[level].last_wave) {
                trigger_load_next_wave();
            } else {
                trigger_win(progress);
            }
        }
    }

    handle_timer(elapsed_ms);
}

void LevelSystem::handle_timer(const float &elapsed_ms) {
    advance_timer = max(0.f, advance_timer - elapsed_ms);

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
}

void LevelSystem::load_wave(int wave) {
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
                BezierPath &path = registry.tilemaps.get(tilemap_system->get_current_map()).patrol_loop[0];
                Entity leader = createEnemy(renderer, path.p0, enemy);
                for (int i = 1; i < 7; i++) {
                    createFollower(renderer, path.p0 + vec2(TILE_SIZE_PX * i, 0.f), leader);
                }
            }
        } else {
            pos = tilemap_system->get_random_position(SpawnTarget::ENEMY);
            createEnemy(renderer, pos, enemy);
        }
    }

    for (auto boss : config.bosses) {
        // TODO: Replace w/ just BOSS
        pos = tilemap_system->get_random_position(SpawnTarget::BOSS | SpawnTarget::ENEMY);
        createBoss(renderer, pos, boss);
    }
}

void LevelSystem::load_level_components() {
    tilemap_system->init_spawn_points();
    auto config = LEVEL_CONFIGS[level];
    vec2 pos;

    for (auto item : config.hazards) {
        pos = tilemap_system->get_random_position(SpawnTarget::HAZARD);
        if (item == HAZARD_TYPE::MINE) {
            createMine(renderer, pos);
        } else if (item == HAZARD_TYPE::ANEMONE) {
            createAnemone(renderer, pos);
        }
    }

    for (auto bubbler : config.bubblers) {
        pos = tilemap_system->get_random_position(SpawnTarget::AMMO);
        createBubbler(renderer, pos);
    }
}

void LevelSystem::load_level(int level) {

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
    Health previous_health;
    FireControl previous_fire_control;
    Wallet previous_wallet;
    Upgrades previous_upgrades;

    if (player_exists && !was_tutorial) {
        Entity current_player = registry.players.entities[0];
        previous_health = registry.healths.get(current_player);
        previous_fire_control = registry.fireControls.get(current_player);
        previous_wallet = registry.wallets.get(current_player);
        previous_upgrades = registry.upgrades.get(current_player);
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

    if (player_exists && !was_tutorial) {
        Entity current_player = registry.players.entities[0];
        registry.healths.get(current_player) = previous_health;
        registry.fireControls.get(current_player) = previous_fire_control;
        registry.wallets.get(current_player) = previous_wallet;
        registry.upgrades.get(current_player) = previous_upgrades;
    }
}

void LevelSystem::load_tutorial() {
    level = -1;
    wave = -1;
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
    player_fc.ammo = 0;

    createEnemy(renderer, {150.f, 200.f}, ENEMY_TYPE::GUARD);
    createEnemy(renderer, {850.f, 200.f}, ENEMY_TYPE::GUARD);
    createEnemy(renderer, {500.f, 400.f}, ENEMY_TYPE::GUARD);

    // Set enemy health to something low for tutorial
    for (Entity e : registry.enemies.entities) {
        Health &h = registry.healths.get(e);
        h.hp = 50;
        h.max_hp = 50;
    }

    createBubbler(renderer, {625.f, 875.f});
    createBubbler(renderer, {425.f, 400.f});

    createMine(renderer, {450.f, 1150.f});
    createPowerUp(renderer, {900.f, 1300.f}, POWER_UP_TYPE::HEAL);

    createTutorialTrigger(player_motion.position, vec2(50.f, 50.f), Tutorial::MOVEMENT);
    createTutorialTrigger(vec2(250.f, 1125.f), vec2(50.f, 350.f), Tutorial::HAZARDS);
    createTutorialTrigger(vec2(600.f, 1150.f), vec2(50.f, 450.f), Tutorial::HEALING);
    createTutorialTrigger(vec2(750.f, 875.f), vec2(400.f, 150.f), Tutorial::AMMUNITION);
    createTutorialTrigger(vec2(750.f, 725.f), vec2(400.f, 50.f), Tutorial::COMBAT);

    camera_system->init(tilemap_system->get_current_map()); // Must be done last, requires player to exist
}

void LevelSystem::restart_game() {
    bool is_tutorial = registry.progresses.components[0].is_tutorial;

    particle_system.clear_all_particles();
    registry.clear_all_components();
    registry.progresses.emplace(Entity("Progress"));

    if (is_tutorial) {
        load_tutorial();
    } else {
        level = 0;
        wave = -1;
        load_level(0);
        trigger_load_next_wave();
    }
}

bool LevelSystem::should_advance() {
    if (registry.progresses.size() > 0) {
        Progress &progress = registry.progresses.components[0];
        if (progress.trigger_skip_wave) {
            progress.trigger_skip_wave = false;
            return true;
        }
    }
    return advance_timer == 0.f && registry.enemies.size() == 0;
}

void LevelSystem::load_next_level() {
    triggered_load_level = false;
    level++;
    load_level(level);
    trigger_load_next_wave();
}

void LevelSystem::load_next_wave() {
    triggered_load_wave = false;
    wave++;
    load_wave(wave);
}

void LevelSystem::trigger_load_next_level() {
    triggered_load_level = true;
    advance_timer = 2000.f;
    if (level == -1) {
        createCompletionText("TUTORIAL COMPLETE!!");
    } else {
        createCompletionText("LEVEL " + std::to_string(level + 1) + " COMPLETE!!");
    }
}

void LevelSystem::trigger_load_next_wave() {
    createCompletionText("LOADING WAVE: " + std::to_string(wave + 1));
    triggered_load_wave = true;
    advance_timer = 2000.f;
}

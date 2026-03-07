#include "ai_system.hpp"

void AISystem::init(PathfindingSystem *system) { ps = system; }

AIAction evaluate_tree(Entity e, const ScriptedTree &tree) {
    if (tree.root_id == -1) {
        return AIAction::IDLE;
    }
    int current_id = tree.root_id;

    while (true) {
        if (tree.nodes.find(current_id) == tree.nodes.end()) {
            return AIAction::IDLE;
        }

        const TreeNode &node = tree.nodes.at(current_id);

        if (node.type == NodeType::ACTION) {
            return node.action;
        }

        bool result = false;
        if (node.type == NodeType::CONDITION) {
            Sense &sense = registry.senses.get(e);
            if (node.condition_name == "sees_player") {
                result = sense.detects_player;
            } else if (node.condition_name == "is_distracted") {
                result = sense.is_distracted;
            } else if (node.condition_name == "is_patrolling") {
                result = sense.is_patrolling;
            } else {
                spdlog::debug("Unknown condition: {}", node.condition_name);
            }
        }
        if (node.type == NodeType::RANDOM) {
            float random = get_random_float(0.f, 1.f);
            result = (random < node.probability);
        }

        current_id = result ? node.true_child : node.false_child;
    }
}

float adjust_angle(Motion &motion, SpeedControl &speed, vec2 target_pos, vec2 player_pos, float elapsed_ms,
                   float stopping_dist) {
    vec2 position = motion.position;
    float angle = motion.angle;

    // TODO: not hardcode stopping distance
    if (entity_in_radius(player_pos - motion.position, motion.scale, stopping_dist)) {
        target_pos = player_pos;
        motion.velocity = vec2(0.f, 0.f);
    } else {
        motion.velocity = vec2(speed.move_speed, 0.f);
    }

    // Adjust angle to track target
    float target_angle_radians = atan2(target_pos.y - position.y, target_pos.x - position.x);
    float max_turn = speed.turn_speed * (elapsed_ms / 1000.f);

    float current_angle = motion.angle;
    float diff = target_angle_radians - current_angle;
    while (diff > M_PI) {
        diff -= 2 * M_PI;
    }
    while (diff < -M_PI) {
        diff += 2 * M_PI;
    }

    float step = std::clamp(diff, -max_turn, max_turn);

    current_angle += step;

    if (current_angle >= 2 * M_PI) {
        current_angle -= 2 * M_PI;
    }
    if (current_angle < 0.f) {
        current_angle += 2 * M_PI;
    }
    motion.angle = current_angle;
    return motion.angle;
}

void fire_projectiles(Entity enemy, float elapsed_ms, float angle) {
    FireControl &fire = registry.fireControls.get(enemy);
    fire.cooldown_timer += elapsed_ms;
    vec2 position = registry.motions.get(enemy).position;

    if (fire.cooldown_timer >= fire.cooldown_ms) {
        if (fire.attack_pattern == ATTACK_PATTERN::URCHIN) {
            float incAngle = 0.f;
            for (int i = 0; i < 8; i++) {
                auto &req = registry.bullet_requests.emplace_with_duplicates(enemy);
                req.attack_pattern = fire.attack_pattern;
                req.position = position;
                req.angle = incAngle;
                req.owner = enemy;
                incAngle += M_PI / 4.f;
            }
        } else if (fire.attack_pattern == ATTACK_PATTERN::SHOTGUN) {
            float incAngle = -2 * M_PI / 12.f;
            for (int i = 0; i < 5; i++) {
                auto &req = registry.bullet_requests.emplace_with_duplicates(enemy);
                req.attack_pattern = fire.attack_pattern;
                req.position = position;
                req.angle = angle + incAngle;
                req.owner = enemy;
                incAngle += M_PI / 12.f;
            }
        } else {
            registry.bullet_requests.insert(enemy, {fire.attack_pattern, position, angle, enemy});
        }
        fire.cooldown_timer = 0;
    }
}

void AISystem::step(float elapsed_ms) {
    Entity player = registry.players.entities[0];
    vec2 player_pos = registry.motions.get(player).position;

    for (uint i = 0; i < registry.ais.components.size(); i++) {
        Entity e = registry.ais.entities[i];
        AI &ai = registry.ais.components[i];

        Motion &motion = registry.motions.get(e);

        ai.current_state = evaluate_tree(e, ai.tree);

        act(e, elapsed_ms, player_pos);
    }
}

void AISystem::run_patrol(Entity e, float elapsed_ms) {
    registry.senses.get(e).is_patrolling = true;
    if (!registry.beziers.has(e)) {
        return;
    }

    Motion &motion = registry.motions.get(e);
    update_leader(e, motion, elapsed_ms);
    if (!registry.senses.get(e).is_patrolling) {
        registry.beziers.get(e).curves_left = 6;
    }

    float dt = elapsed_ms / 1000.f;

    for (Entity follower_e : registry.boids.entities) {
        if (!registry.motions.has(follower_e) || registry.boids.get(follower_e).leader != e) {
            continue;
        }

        update_follower(follower_e, e, elapsed_ms);
    }
}

void AISystem::run_pursuit(Entity e, float elapsed_ms, vec2 player_pos) {
    Motion &motion_i = registry.motions.get(e);
    vec2 target_pos = player_pos;
    float angle = 0;

    // Pathfinding
    if (registry.pursuits.has(e)) {
        Pursuit &pursuit = registry.pursuits.get(e);
        if (pursuit.goal != world_to_tile(player_pos) || pursuit.path.size() == 0) {
            pursuit.path = ps->path_finding(motion_i.position, target_pos);
            pursuit.curr_index = 0;
            pursuit.goal = world_to_tile(player_pos);
        }

        if (pursuit.curr_index < pursuit.path.size() && pursuit.path.size() > 0) {
            vec2 next_pos = pursuit.path[pursuit.curr_index];

            if (length(next_pos - motion_i.position) < (TILE_SIZE_PX / 2)) {
                if (pursuit.curr_index + 1 < pursuit.path.size()) {
                    pursuit.curr_index += 1;
                    next_pos = pursuit.path[pursuit.curr_index];
                }
            }

            target_pos = next_pos;
        }

        angle = adjust_angle(motion_i, registry.speedControls.get(e), target_pos, player_pos, elapsed_ms,
                             (TILE_SIZE_PX * 1.5));
    }

    // Firing logic
    if (ps->walkable_bresenhams_line(world_to_tile(motion_i.position), world_to_tile(player_pos))) {
        fire_projectiles(e, elapsed_ms, angle);
    }
}

void AISystem::run_distracted(Entity e, float elapsed_ms, vec2 player_pos) {
    if (!registry.distracteds.has(e)) {
        return;
    }
    Distracted &distracted = registry.distracteds.get(e);
    distracted.remaining_ms -= elapsed_ms;
    float distracted_remaining = distracted.remaining_ms;

    if (distracted.remaining_ms > 0) {
        Motion &motion_i = registry.motions.get(e);
        vec2 bell_pos = registry.motions.get(distracted.bell).position;
        adjust_angle(motion_i, registry.speedControls.get(e), bell_pos, player_pos, elapsed_ms, 50.f);
    } else {
        registry.senses.get(e).is_distracted = false;
    }
}

void AISystem::run_idle(Entity e) {
    Motion &motion = registry.motions.get(e);
    motion.velocity = vec2(0.f, 0.f);
}

void AISystem::act(Entity e, float elapsed_ms, vec2 player_pos) {
    AI &ai = registry.ais.get(e);

    switch (ai.current_state) {
    case AIAction::PURSUE:
        run_pursuit(e, elapsed_ms, player_pos);
        break;

    case AIAction::IDLE:
        run_idle(e);
        break;

    case AIAction::DISTRACTED:
        run_distracted(e, elapsed_ms, player_pos);
        break;

    case AIAction::PATROL:
        run_patrol(e, elapsed_ms);
        break;
    }
}

void AISystem::handle_detection() {
    ComponentContainer<Enemy> &enemy_container = registry.enemies;
    Entity player = registry.players.entities[0];
    vec2 player_pos = registry.motions.get(player).position;

    for (uint i = 0; i < enemy_container.components.size(); i++) {
        Entity enemy_i = enemy_container.entities[i];
        Sense &sense_i = registry.senses.get(enemy_i);

        if (sense_i.last_known_player_pos != player_pos) {
            float radius_i = enemy_container.components[i].detection_radius;
            Motion &motion_i = registry.motions.get(enemy_i);

            sense_i.detects_player = entity_in_radius(player_pos - motion_i.position, motion_i.scale, radius_i);

            if (sense_i.detects_player) {
                sense_i.last_known_player_pos = player_pos;
                if (registry.pursuits.has(enemy_i)) {
                    registry.pursuits.get(enemy_i).goal = player_pos;
                }

                // spdlog::debug("{} detected {}", enemy_i.name_id(), player.name_id());
            }
        }
    }
}

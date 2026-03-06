#include "game_ui.hpp"

bool can_afford_upgrade(const UPGRADE_TYPE type) {
    if (registry.players.entities.size() > 0) {
        Entity &player = registry.players.entities[0];
        if (registry.upgrades.has(player) && registry.wallets.has(player)) {
            UpgradeInstance &upgrade_instance = registry.upgrades.get(player).upgrade_instances[type];
            unsigned int curr_level = upgrade_instance.level;
            unsigned int &num_crackers = registry.wallets.get(player).crackers;

            if (curr_level < upgrade_instance.config.max_levels - 1) {
                return upgrade_instance.config.cost[curr_level + 1] >= num_crackers;
            }
        }
    }
    return false;
}

void try_upgrade_player(const UPGRADE_TYPE type) {
    if (!can_afford_upgrade(type))
        return;
    if (registry.players.entities.size() > 0) {
        Entity &player = registry.players.entities[0];
        if (registry.upgrades.has(player) && registry.wallets.has(player)) {
            UpgradeInstance &upgrade_instance = registry.upgrades.get(player).upgrade_instances[type];
            unsigned int &curr_level = upgrade_instance.level;
            unsigned int &num_crackers = registry.wallets.get(player).crackers;

            if (curr_level < upgrade_instance.config.max_levels - 1) {
                curr_level++;
                num_crackers -= upgrade_instance.config.cost[curr_level];
                switch (type) {
                    case UPGRADE_TYPE::MAX_AMMO:
                        upgrade_max_ammo(player, upgrade_instance);
                        break;
                    case UPGRADE_TYPE::MAX_HP:
                        upgrade_max_hp(player, upgrade_instance);
                        break;
                    case UPGRADE_TYPE::ATTACK:
                        upgrade_attack(player, upgrade_instance);
                        break;
                }
            }
        }
    }
}


void upgrade_max_ammo(Entity &player, UpgradeInstance &upgrade_instance) {
    if (registry.fireControls.has(player)) {
        FireControl &fc = registry.fireControls.get(player);
        fc.max_ammo += upgrade_instance.config.bonus[upgrade_instance.level];
        fc.ammo     += upgrade_instance.config.bonus[upgrade_instance.level];
    }
}

void upgrade_max_hp(Entity &player, UpgradeInstance &upgrade_instance) {
    if (registry.healths.has(player)) {
        Health& health = registry.healths.get(player);
        health.max_hp += upgrade_instance.config.bonus[upgrade_instance.level];
        health.hp     += upgrade_instance.config.bonus[upgrade_instance.level];
    }
}

void upgrade_attack(Entity &player, UpgradeInstance &upgrade_instance) {
    if (registry.fireControls.has(player)) {
        FireControl &fc = registry.fireControls.get(player);
        fc.damage_bonus += upgrade_instance.config.bonus[upgrade_instance.level];
    }
}
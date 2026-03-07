#pragma once
#include "common.hpp"
#include "configs/upgrade_config.hpp"
#include "tinyECS/registry.hpp"

class UpgradeSystem {
public:
    void try_upgrade_player(UPGRADE_TYPE type);
    void close_upgrade_menu();
    bool can_afford_upgrade(const UPGRADE_TYPE type);

private:
    void upgrade_max_ammo(Entity &player, UpgradeInstance &upgrade_instance);
    void upgrade_max_hp(Entity &player, UpgradeInstance &upgrade_instance);
    void upgrade_attack(Entity &player, UpgradeInstance &upgrade_instance);
};
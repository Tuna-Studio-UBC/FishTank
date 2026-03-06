#pragma once
#include "common.hpp"

enum UPGRADE_TYPE {
    ATTACK,
    MAX_AMMO,
    MAX_HP,
    UPGRADE_COUNT
};

const unsigned int MAX_UPGRADES = 3;

struct UpgradeConfig {
    unsigned int max_levels;
    int bonus[MAX_UPGRADES];
    int cost[MAX_UPGRADES];
};

struct UpgradeInstance {
    unsigned int level = 0;
    UpgradeConfig config;

    UpgradeInstance(const UpgradeConfig config) : config(config) {};
};

const UpgradeConfig ATTACK_UPGRADE_CONFIG = {3, {0, 5, 5}, {0, 0, 0}};
const UpgradeConfig MAX_AMMO_UPGRADE_CONFIG = {3, {0, 10, 10}, {0, 0, 0}};
const UpgradeConfig MAX_HP_UPGRADE_CONFIG = {3, {0, 20, 30}, {0, 0, 0}};
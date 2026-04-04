#pragma once
#include "common.hpp"

enum UPGRADE_TYPE { ATTACK, MAX_AMMO, MAX_HP, UPGRADE_COUNT };

const unsigned int MAX_UPGRADES = 4;

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

const UpgradeConfig ATTACK_UPGRADE_CONFIG = {4, {0, 5, 10, 15}, {0, 10, 20, 30}};
const UpgradeConfig MAX_AMMO_UPGRADE_CONFIG = {4, {0, 10, 20, 30}, {0, 10, 15, 20}};
const UpgradeConfig MAX_HP_UPGRADE_CONFIG = {4, {0, 20, 40, 60}, {0, 5, 10, 15}};
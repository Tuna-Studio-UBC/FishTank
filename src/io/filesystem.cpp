#include "filesystem.hpp"
#include <spdlog/spdlog.h>
#include <string>

Filesystem filesystem;

std::string SAVE_DIR = PROJECT_SOURCE_DIR + std::string("data/");
std::string SAVE_FILE = SAVE_DIR + std::string("data.txt");

// We save:
// - The highest score so far
// - The data in the persistence object (Info from the start of the most recent level)

#define DEBUG_FILESYSTEM

#ifdef DEBUG_FILESYSTEM
void print_persistence() {
    if (registry.persistences.components.size() == 0) {
        spdlog::debug("No persistence object found");
        return;
    }

    Persistence &per = registry.persistences.components[0];

    auto &sco = per.score;
    auto &wal = per.wallet;
    
    auto &up = per.upgrades.upgrade_instances;
    auto &up_attack = up[0];
    auto &up_ammo = up[1];
    auto &up_hp = up[2];

    spdlog::debug("\n[Persistence]\nLevel {} Wave {}\nWallet: {}\nScore: {} {} {} {} {}\nUpgrades: {} {} {}",
        per.level, per.wave,
        wal.crackers,
        sco.been_hit, sco.bullets_fired, sco.bullets_hit, sco.crackers_earned, sco.enemies_defeated,
        up_attack.level, up_ammo.level, up_hp.level
    );
}
#endif

void Filesystem::load() {
    spdlog::info("Loading game info");
    #ifdef DEBUG_FILESYSTEM
    print_persistence();
    #endif
}

void Filesystem::save() {
    spdlog::info("Saving game info");
    #ifdef DEBUG_FILESYSTEM
    print_persistence();
    #endif
}
#include "persist_file.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <string>

PersistFile persist_file;

std::string SAVE_DIR = PROJECT_SOURCE_DIR + std::string("data/");
std::string SAVE_FILE = SAVE_DIR + std::string("data.txt");
std::string SCORE_FILE = SAVE_DIR + std::string("score.txt");

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

void ensure_directory_exists() {
    try {
        if (std::filesystem::create_directories(SAVE_DIR)) {
            spdlog::info("Created directory {}", SAVE_DIR);
        } else {
            spdlog::info("Data directory exists");
        }
    } catch (const std::filesystem::filesystem_error &e) {
        spdlog::error("Error while creating directory {}", SAVE_DIR);
    }
}

void PersistFile::load() {
    spdlog::info("Loading game info");

    #ifdef DEBUG_FILESYSTEM
    print_persistence();
    #endif
}

void PersistFile::save() {
    spdlog::info("Saving game info");

    #ifdef DEBUG_FILESYSTEM
    print_persistence();
    #endif

    if (registry.persistences.components.size() == 0) {
        spdlog::debug("No persistence object found");
        return;
    }

    Persistence &per = registry.persistences.components[0];

    auto &wal = per.wallet;
    auto &sco = per.score;

    auto &up = per.upgrades.upgrade_instances;
    auto &up_attack = up[0];
    auto &up_ammo = up[1];
    auto &up_hp = up[2];

    ensure_directory_exists();

    std::ofstream outputFile(SAVE_FILE);
    if (!outputFile.is_open()) {
        spdlog::error("Error: Could not open the file for saving");
        return;
    }

    outputFile << per.level << " " << per.wave << std::endl;
    outputFile << wal.crackers << std::endl;
    outputFile << sco.been_hit << " " << sco.bullets_fired << " " << sco.bullets_hit << " " <<  sco.crackers_earned << " " << sco.enemies_defeated << std::endl;
    outputFile << up_attack.level << " " << up_ammo.level << " " << up_hp.level << std::endl;

    outputFile.close();

    spdlog::info("Saved game data");
}
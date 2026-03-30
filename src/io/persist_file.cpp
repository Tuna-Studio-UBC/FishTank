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

    std::ifstream inputFile(SAVE_FILE);
    if (!inputFile.is_open()) {
        spdlog::error("Error: Could not open the file for loading");
        return;
    }

    // Reading the various expected parts
    std::string line;

    int level, wave;
    if(std::getline(inputFile, line)) {
        std::stringstream ss_line(line);
        ss_line >> level >> wave;
    } else {
        spdlog::error("Error: Could not read first line of file");
        return;
    }

    unsigned int crackers;
    if (std::getline(inputFile, line)) {
        std::stringstream ss_line(line);
        ss_line >> crackers;
    } else {
        spdlog::error("Error: Could not read second line of file");
        return;
    }

    int been_hit, bullets_fired, bullets_hit, crackers_earned, enemies_defeated;
    if (std::getline(inputFile, line)) {
        std::stringstream ss_line(line);
        ss_line >> been_hit >> bullets_fired >> bullets_hit >> crackers_earned >> enemies_defeated;
    } else {
        spdlog::error("Error: Could not read third line of file");
        return;
    }

    unsigned int attack_level, ammo_level, hp_level;
    if (std::getline(inputFile, line)) {
        std::stringstream ss_line(line);
        ss_line >> attack_level >> ammo_level >> hp_level;
    } else {
        spdlog::error("Error: Could not read fourth line of file");
        return;
    }

    spdlog::info("Loading data:\n{} {}\n{}\n{} {} {} {} {}\n{} {} {}", level, wave, crackers, been_hit, bullets_fired,
                 bullets_hit, crackers_earned, enemies_defeated, attack_level, ammo_level, hp_level);

    if (registry.persistences.components.size() == 0) {
        spdlog::debug("No persistence object found");
        return;
    }

    Persistence &per = registry.persistences.components[0];
    
    per.level = level;
    per.wave = wave;

    per.wallet.crackers = crackers;

    per.score.been_hit = been_hit;
    per.score.bullets_fired = bullets_fired;
    per.score.bullets_hit = bullets_hit;
    per.score.crackers_earned = crackers_earned;
    per.score.enemies_defeated = enemies_defeated;

    per.upgrades.upgrade_instances[0].level = attack_level;
    per.upgrades.upgrade_instances[1].level = ammo_level;
    per.upgrades.upgrade_instances[2].level = hp_level;

    if (registry.progresses.components.size() == 0) {
        spdlog::debug("No progress object found");
        return;
    }

    Progress &pro = registry.progresses.components[0];

    pro.trigger_restart = true;
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
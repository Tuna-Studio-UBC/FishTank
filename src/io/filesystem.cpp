#include "filesystem.hpp"
#include <spdlog/spdlog.h>
#include <string>

Filesystem filesystem;

std::string SAVE_DIR = PROJECT_SOURCE_DIR + std::string("data/");
std::string SAVE_FILE = SAVE_DIR + std::string("data.txt");

void Filesystem::load() {
    spdlog::info("Loading game info");
}

void Filesystem::save() {
    spdlog::info("Saving game info");
}
#pragma once

#include "tinyECS/registry.hpp"

class PersistFile {
    public:
        PersistFile() {};

        void save();

        void load();
};

extern PersistFile persist_file;
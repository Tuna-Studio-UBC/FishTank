#pragma once

#include "tinyECS/registry.hpp"

class Filesystem {
    public:
        Filesystem() {};

        void save();

        void load();
};

extern Filesystem filesystem;
#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class BulletSystem
{
public:
	void step(float elapsed_ms);
};
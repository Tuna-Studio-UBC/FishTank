#pragma once
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "../common.hpp"

class CameraSystem
{
public:
	void step();

	CameraSystem () {}
};
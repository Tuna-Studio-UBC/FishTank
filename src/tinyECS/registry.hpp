#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface *> registry_list;

public:
	// Entity
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Bullet> bullet;
	ComponentContainer<Wall> wall;
	ComponentContainer<Mine> mine;

	// Behaviour
	ComponentContainer<Health> health;
	ComponentContainer<FireControl> fireControl;
	ComponentContainer<SpeedControl> speedControl;
	ComponentContainer<PlayerQuickTurn> playerQuickTurn;
	ComponentContainer<Wallet> wallet;
	ComponentContainer<DropsCrackers> dropsCrackers;
	ComponentContainer<Patrolling> patrolling;
	ComponentContainer<Pursuit> pursuit;
	ComponentContainer<Bouncy> bouncy;

	// Physics
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;

	// Game State
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<KeyboardInputState> keyboardInput;
	ComponentContainer<MouseInputState> mouseInput;

	// Rendering
	ComponentContainer<Mesh *> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<vec3> colors;
	ComponentContainer<TileSet> tilesets;
	ComponentContainer<TileMap> tilemaps;
	
	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		registry_list.push_back(&motions);
		// Entity
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&bullet);
		registry_list.push_back(&wall);
		registry_list.push_back(&mine);

		// Behaviour
		registry_list.push_back(&health);
		registry_list.push_back(&fireControl);
		registry_list.push_back(&speedControl);
		registry_list.push_back(&playerQuickTurn);
		registry_list.push_back(&wallet);
		registry_list.push_back(&dropsCrackers);
		registry_list.push_back(&patrolling);
		registry_list.push_back(&pursuit);
		registry_list.push_back(&bouncy);

		// Physics
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);

		// Game State
		registry_list.push_back(&screenStates);
		registry_list.push_back(&keyboardInput);
		registry_list.push_back(&mouseInput);

		// Rendering
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&colors);
		registry_list.push_back(&tilesets);
		registry_list.push_back(&tilemaps);
	}

	void clear_all_components()
	{
		for (ContainerInterface *reg : registry_list)
			reg->clear();
	}

	void list_all_components()
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface *reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e)
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface *reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e)
	{
		for (ContainerInterface *reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;
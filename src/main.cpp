#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "systems/collision_system.hpp"
#include "systems/ote_system.hpp"
#include "systems/heal_damage_system.hpp"
#include "systems/death_system.hpp"
#include "systems/pick_up_system.hpp"
#include "systems/player_input_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem ai_system;
	CollisionSystem collision_system;
	OverTimeEffectSystem ote_system;
	HealDamageSystem heal_damage_system;
	DeathSystem death_system;
	PickUpSystem pick_up_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;
	PlayerInputSystem player_input_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	//Commented out duo to compiler errors
	/*if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}*/

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		if (world_system.step(elapsed_ms)) {
			ote_system.step(elapsed_ms);
			heal_damage_system.step(elapsed_ms);
			death_system.step(elapsed_ms);
			pick_up_system.step(elapsed_ms);
			ai_system.handle_detection();
			ai_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			player_input_system.step(elapsed_ms);
			world_system.handle_expiration(elapsed_ms);
			collision_system.step(elapsed_ms);
		}

		renderer_system.draw();
	}

	return EXIT_SUCCESS;
}

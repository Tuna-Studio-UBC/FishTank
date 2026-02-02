// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem()
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	// if (background_music != nullptr)
	// 	Mix_FreeMusic(background_music);
	// if (invader_hit_sound != nullptr)
	// 	Mix_FreeChunk(invader_hit_sound);
	// if (tower_hit_sound != nullptr)
	// 	Mix_FreeChunk(tower_hit_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	/*background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	invader_hit_sound = Mix_LoadWAV(audio_path("invader_hit.wav").c_str());
	tower_hit_sound = Mix_LoadWAV(audio_path("tower_hit.wav").c_str());

	if (background_music == nullptr || invader_hit_sound == nullptr || tower_hit_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("invader_hit.wav").c_str(),
			audio_path("tower_hit.wav").c_str());
		return false;
	}*/

	return true;
}

void WorldSystem::init(RenderSystem* renderer_arg) {

	this->renderer = renderer_arg;

	init_tiles();

	// Create player entity
	Entity player = createPlayer(renderer, { WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX / 2.0f });
	// start playing background music indefinitely

	//Commented out duo to compiler errors
	/*std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);*/
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Game: ";
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	return true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	// TODO
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		close_window();
	}

	// We should have 1 and only ever 1 player input component
	assert(registry.playerInput.components.size() == 1);
	PlayerInputState &ps = registry.playerInput.components[0];

	bool key_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

	// We handle the inputs separately since we need to deal with game logic
	// E.g. how to handle multiple conflicting inputs
	if (key == GLFW_KEY_W) {
		ps.current_actions[PlayerActions::MoveForward] = key_pressed;
	}
	if (key == GLFW_KEY_A) {
		ps.current_actions[PlayerActions::TurnLeft] = key_pressed;
	}
	if (key == GLFW_KEY_D) {
		ps.current_actions[PlayerActions::TurnRight] = key_pressed;
	}
	if (key == GLFW_KEY_S) {
		ps.current_actions[PlayerActions::QuickTurn] = key_pressed;
	}

	// Debug
	if (key == GLFW_KEY_P) {
		registry.list_all_components();
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			vec2 mouse_pos = vec2(mouse_pos_x, mouse_pos_y);
			Motion& player_motion = registry.motions.get(registry.players.entities[0]);
			vec2& player_pos = player_motion.position;
			// https://stackoverflow.com/questions/2339487/calculate-angle-of-2-points
			float angle = atan2(mouse_pos.y - player_pos.y, mouse_pos.x - player_pos.x);
			createProjectile(renderer, player_pos, angle);


			// std::cout << "Mouse Pos: (" << lround(mouse_pos.x) << ", " << lround(mouse_pos.y) << ")\n";
			// std::cout << "Player Pos: (" << lround(player_pos.x) << ", " << lround(player_pos.y) << ")\n";
			// std::cout << "Angle: " << angle << "\n";
		}
	}
}
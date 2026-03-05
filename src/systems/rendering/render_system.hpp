#pragma once

#include <array>
#include <map>
#include <utility>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// font character structure
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    ivec2 Size;             // Size of glyph
    ivec2 Bearing;          // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Offset to advance to next glyph
    char character;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
    /**
     * The following arrays store the assets the game will use. They are loaded
     * at initialization and are assumed to not be modified by the render loop.
     *
     * Whenever possible, add to these lists instead of creating dynamic state
     * it is easier to debug and faster to execute for the computer.
     */
    std::array<GLuint, texture_count> texture_gl_handles;
    std::array<ivec2, texture_count> texture_dimensions;

    // Font characters
    std::map<char, Character> ft_characters;

    // Make sure these paths remain in sync with the associated enumerators.
    // Associated id with .obj path
    const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN,
                                                   get_path(FilePath::MESHES, "chicken.obj"))
        // specify meshes of other assets here
    };

    // Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
    const std::array<std::string, texture_count> texture_paths = {
        // Tileset images
        get_path(FilePath::TILESET_IMAGES, "greybox.png"), get_path(FilePath::TILESET_IMAGES, "greybox2.png"),

        // Textures
        get_path(FilePath::TEXTURES, "white_bubble.png"), get_path(FilePath::TEXTURES, "black.png"),
        get_path(FilePath::TEXTURES, "tutorial_complete.png"), get_path(FilePath::TEXTURES, "win.png"),
        get_path(FilePath::TEXTURES, "lose.png"), get_path(FilePath::TEXTURES, "bubble.png"),
        get_path(FilePath::TEXTURES, "bubbler.png"),

        // Effects
        get_path(FilePath::TEXTURES, "smoke.png"),

        // Sprite sheets
        get_path(FilePath::SPRITE_SHEETS, "bubble_sheet.png"), get_path(FilePath::SPRITE_SHEETS, "fish_sheet.png"),
        get_path(FilePath::SPRITE_SHEETS, "turret_sheet.png"), get_path(FilePath::SPRITE_SHEETS, "crab_sheet.png"),
        get_path(FilePath::SPRITE_SHEETS, "health_sheet.png"), get_path(FilePath::SPRITE_SHEETS, "mine_sheet.png"),
        get_path(FilePath::SPRITE_SHEETS, "urchin_sheet.png"), get_path(FilePath::SPRITE_SHEETS, "isopod_sheet.png")};

    std::array<GLuint, effect_count> effects;
    // Make sure these paths remain in sync with the associated enumerators.
    const std::array<std::string, effect_count> effect_paths = {
        get_path(FilePath::SHADERS, "coloured"), get_path(FilePath::SHADERS, "egg"),
        get_path(FilePath::SHADERS, "chicken"),  get_path(FilePath::SHADERS, "textured"),
        get_path(FilePath::SHADERS, "font"),     get_path(FilePath::SHADERS, "vignette"),
        get_path(FilePath::SHADERS, "tilemap"),  get_path(FilePath::SHADERS, "particle")};

    std::array<GLuint, geometry_count> vertex_buffers;
    std::array<GLuint, geometry_count> index_buffers;
    std::array<Mesh, geometry_count> meshes;

  public:
    // Initialize the window
    bool init(GLFWwindow *window);

    bool initializeFonts(GLFWwindow *window, const std::string &font_filename, unsigned int font_default_size = 24);

    void initializeGUI(GLFWwindow *window);

    template <class T>
    void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

    void initializeGlTextures();

    void initializeGlEffects();

    void initializeGlMeshes();

    void initializeParticles();

    Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

    void initializeGlGeometryBuffers();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture, then it is used for the vignette shader
    bool initScreenTexture();

    // Destroy resources associated to one or all entities created by the system
    ~RenderSystem();

    // Draw all entities
    void draw();

    void renderGUI();

    void shutdown();

    mat3 createProjectionMatrix();
    mat3 createViewMatrix();

    Entity get_screen_state_entity() { return screen_state_entity; }

  private:
    // Internal drawing functions for each entity type
    void drawTileMap(const Entity &e, const mat3 &projection, const mat3 &view);
    void drawParticles(const mat3 &projection, const mat3 &view);
    void drawTexturedMesh(Entity entity, const mat3 &projection, const mat3 &view);

    // On-screen rendering
    void drawToScreen();
    void drawText(Entity entity);

    void sortRenderRequests(std::vector<Entity> *draw_list);

    // Window handle
    GLFWwindow *window;

    // Screen texture handles
    GLuint frame_buffer;
    GLuint off_screen_render_buffer_color;
    GLuint off_screen_render_buffer_depth;

    GLuint vao;

    // For instancing
    GLuint particle_vao;
    GLuint quad_vbo;
    GLuint instance_vbo;
    unsigned int instance_buffer_capacity = 1000;

    GLuint font_vao;
    GLuint font_vbo;

    Entity screen_state_entity;
};

bool loadEffectFromFile(const std::string &vs_path, const std::string &fs_path, GLuint &out_program);

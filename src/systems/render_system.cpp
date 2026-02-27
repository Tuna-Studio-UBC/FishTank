
#include <SDL.h>
#include <iostream>
#include <queue>
#include <spdlog/spdlog.h>

// internal
#include "systems/particle_system.hpp"
#include "systems/render_system.hpp"
#include "tinyECS/registry.hpp"

void draw_tile(TileSet &tileset, const TileMap &tilemap, const ivec2 &tile_pos, const GLuint program) {
    Transform transform;
    transform.translate((TILE_SIZE_PX * tile_pos) + (TILE_SIZE_PX / 2));
    transform.scale(vec2(TILE_SIZE_PX));

    unsigned short tile_id = tilemap.tile_ids[GET_2D_POS(tile_pos.x, tile_pos.y, tilemap.width)];
    if (tile_id == 0)
        return;

    tile_id--;

    int tileset_width = tileset.dimensions.x;
    int tileset_height = tileset.dimensions.y;

    // Atlas offset is normalized atlas coordinates
    vec2 atlas_offset =
        vec2((tile_id % tileset_width) / (float)tileset_width, (tile_id / tileset_width) / (float)tileset_height);
    GLint atlas_offset_uloc = glGetUniformLocation(program, "atlas_offset");
    glUniform2fv(atlas_offset_uloc, 1, value_ptr(atlas_offset));
    gl_has_errors();

    GLuint transform_loc = glGetUniformLocation(program, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, value_ptr(transform.mat));
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();
    GLsizei num_indices = size / sizeof(uint16_t);
    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawTileMap(const Entity &entity, const mat3 &projection, const mat3 &view) {
    RenderRequest &render_request = registry.renderRequests.get(entity);
    TileMap &tilemap = registry.tilemaps.get(entity);
    TileSet &tileset = registry.tilesets.get(entity);
    vec2 &camera_pos = registry.motions.get(registry.camera.entities[0]).position;
    // Only render the tiles that are on screen. Use camera pos and not player pos
    int min_x = max(((int)(camera_pos.x - (WINDOW_WIDTH_PX / 2)) / TILE_SIZE_PX) - 1, 0);
    int max_x = min(((int)(camera_pos.x + (WINDOW_WIDTH_PX / 2)) / TILE_SIZE_PX) + 1, (int)tilemap.width);
    int min_y = max(((int)(camera_pos.y - (WINDOW_HEIGHT_PX / 2)) / TILE_SIZE_PX) - 1, 0);
    int max_y = min(((int)(camera_pos.y + (WINDOW_HEIGHT_PX / 2)) / TILE_SIZE_PX) + 1, (int)tilemap.height);

    const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TILEMAP];
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
    gl_has_errors();
    assert(in_texcoord_loc >= 0);

    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
    gl_has_errors();

    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

    // Enabling and binding texture to slot 0
    glActiveTexture(GL_TEXTURE0);
    gl_has_errors();

    GLuint texture_id = texture_gl_handles[(GLuint)render_request.used_texture];

    glBindTexture(GL_TEXTURE_2D, texture_id);
    gl_has_errors();

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = vec3(1);
    glUniform3fv(color_uloc, 1, value_ptr(color));
    gl_has_errors();

    GLint tileset_size_uloc = glGetUniformLocation(program, "tileset_size");
    glUniform2fv(tileset_size_uloc, 1, value_ptr(tileset.dimensions));
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(program, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, value_ptr(projection));
    gl_has_errors();

    GLuint view_loc = glGetUniformLocation(program, "view");
    glUniformMatrix3fv(view_loc, 1, GL_FALSE, value_ptr(view));
    gl_has_errors();

    GLuint tile_size_loc = glGetUniformLocation(program, "tile_size");
    glUniform1f(tile_size_loc, tileset.tile_width_px);
    gl_has_errors();

    // https://learnopengl.com/Getting-started/Textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (int y = min_y; y < max_y; y++)
        for (int x = min_x; x < max_x; x++)
            draw_tile(tileset, tilemap, ivec2(x, y), program);
}

void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection, const mat3 &view) {
    Motion &motion = registry.motions.get(entity);
    // Transformation code, see Rendering and Transformation in the template
    // specification for more info Incrementally updates transformation matrix,
    // thus ORDER IS IMPORTANT
    Transform transform;
    transform.translate(motion.position);
    transform.rotate(motion.angle);
    transform.scale(motion.scale);

    assert(registry.renderRequests.has(entity));
    const RenderRequest &render_request = registry.renderRequests.get(entity);

    const GLuint used_effect_enum = (GLuint)render_request.used_effect;
    assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint)effects[used_effect_enum];

    // Setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // texture-mapped entities - use data location as in the vertex buffer
    if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED) {
        vec2 uv_offset = {0.f, 0.f};
        vec2 uv_scale = {1.f, 1.f};

        // Reuse the textured mesh shader so that we don't need custom functionality for animations
        if (registry.animations.has(entity)) {
            Animation &an = registry.animations.get(entity);

            float frame_width = 1.f / an.columns;
            float frame_height = 1.f / an.rows;

            int col = an.current_frame % an.columns;
            int row = an.current_frame / an.columns;

            uv_offset = {col * frame_width, row * frame_height};
            uv_scale = {frame_width, frame_height};
        }

        GLint offset_loc = glGetUniformLocation(program, "uv_offset");
        GLint scale_loc = glGetUniformLocation(program, "uv_scale");
        glUniform2fv(offset_loc, 1, value_ptr(uv_offset));
        glUniform2fv(scale_loc, 1, value_ptr(uv_scale));
        gl_has_errors();

        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
        gl_has_errors();
        assert(in_texcoord_loc >= 0);

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
        gl_has_errors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                              (void *)sizeof(vec3)); // note the stride to skip the preceeding vertex position

        // Enabling and binding texture to slot 0
        glActiveTexture(GL_TEXTURE0);
        gl_has_errors();

        assert(registry.renderRequests.has(entity));
        GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

        glBindTexture(GL_TEXTURE_2D, texture_id);
        gl_has_errors();
    }
    // .obj entities
    else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN ||
             render_request.used_effect == EFFECT_ASSET_ID::EGG) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)sizeof(vec3));
        gl_has_errors();
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
    glUniform3fv(color_uloc, 1, value_ptr(color));
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);
    // GLsizei num_triangles = num_indices / 3;

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, value_ptr(transform.mat));
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, value_ptr(projection));
    gl_has_errors();

    GLuint view_loc = glGetUniformLocation(currProgram, "view");
    glUniformMatrix3fv(view_loc, 1, GL_FALSE, value_ptr(view));
    gl_has_errors();

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawText(Entity entity) {
    Text &t = registry.text.get(entity);
    std::string text = t.text;
    float x = t.position.x;
    float y = t.position.y;
    float scale = t.scale;
    const glm::vec3 &color = t.color;
    const glm::mat4 &trans = t.trans;

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::FONT];
    glUseProgram(program);

    GLint transform_loc = glGetUniformLocation(program, "transform");
    assert(transform_loc > -1);
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, value_ptr(trans));

    GLint textColor_loc = glGetUniformLocation(program, "textColor");
    assert(textColor_loc > -1);
    glUniform3f(textColor_loc, color.r, color.g, color.b);

    glBindVertexArray(font_vao);

    // iterate through characters of the text string
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = ft_characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

        // render glyph texture over quad

        // bind current char texture
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance position for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(vao);
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen() {
    // Setting shaders
    // get the vignette texture, sprite mesh, and program
    glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE]);
    gl_has_errors();

    // Clearing backbuffer
    int w, h;
    glfwGetFramebufferSize(window, &w,
                           &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(1.f, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl_has_errors();
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Draw the screen texture on the quad geometry
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
                                                                     // indices to the bound GL_ARRAY_BUFFER
    gl_has_errors();

    // add the "vignette" effect
    const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];

    // set clock
    GLuint time_uloc = glGetUniformLocation(vignette_program, "time");
    GLuint dead_timer_uloc = glGetUniformLocation(vignette_program, "darken_screen_factor");

    glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));

    if (!registry.screenStates.has(screen_state_entity)) {
        registry.screenStates.emplace(screen_state_entity);
    }
    ScreenState &screen = registry.screenStates.get(screen_state_entity);
    // spdlog::debug("screen.darken_screen_factor: {} entity id: {}", screen.darken_screen_factor,
    //               screen_state_entity);
    glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
    gl_has_errors();

    // Set the vertex position and vertex texture coordinates (both stored in the
    // same VBO)
    GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
    gl_has_errors();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    gl_has_errors();

    // Draw
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
                   nullptr); // one triangle = 3 vertices; nullptr indicates that there is
                             // no offset from the bound index buffer
    gl_has_errors();
}

void RenderSystem::drawParticles(const mat3 &projection, const mat3 &view) {
    static std::vector<vec2> instance_positions;

    // Load program

    const GLuint program = effects[(GLuint)EFFECT_ASSET_ID::PARTICLE];
    glUseProgram(program);
    gl_has_errors();

    glBindVertexArray(particle_vao);
    gl_has_errors();
    
    for (auto &[entity_id, rt] : particle_system.runtimes) {
        assert(rt.texture != TEXTURE_ASSET_ID::TEXTURE_COUNT);

        if (rt.particles.empty()) {
            continue;
        }

        instance_positions.clear();

        for (auto &p : rt.particles) {
            instance_positions.push_back(p.position);
        }

        // spdlog::debug("Particle count: {}", instance_positions.size());

        if (instance_positions.size() > instance_buffer_capacity) {
            instance_buffer_capacity = instance_positions.size() * 2;

            glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
            glBufferData(GL_ARRAY_BUFFER, instance_buffer_capacity * sizeof(vec2), nullptr, GL_STREAM_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, instance_positions.size() * sizeof(vec2), instance_positions.data());
        gl_has_errors();

        // Texture Items

        glActiveTexture(GL_TEXTURE0);
        GLuint texture_id = texture_gl_handles[(GLuint)rt.texture];
        glBindTexture(GL_TEXTURE_2D, texture_id);
        gl_has_errors();

        // Rendering

        GLuint projection_loc = glGetUniformLocation(program, "projection");
        glUniformMatrix3fv(projection_loc, 1, GL_FALSE, value_ptr(projection));
        gl_has_errors();

        GLuint view_loc = glGetUniformLocation(program, "view");
        glUniformMatrix3fv(view_loc, 1, GL_FALSE, value_ptr(view));
        gl_has_errors();

        GLuint scale_loc = glGetUniformLocation(program, "particle_scale");
        glUniform1f(scale_loc, rt.particle_scale);

        // Draw 6 vertices, instanced
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instance_positions.size());
        gl_has_errors();
    }
    
    glBindVertexArray(vao);
}

// Z-sort render requests; stores draw order of entities in draw_list
void RenderSystem::sortRenderRequests(std::vector<Entity> *draw_list) {
    std::priority_queue<EntityRRPair, std::vector<EntityRRPair>, EntityRRPairCompare> sorted_rr;
    for (int i = 0; i < registry.renderRequests.entities.size(); i++) {
        Entity entity = registry.renderRequests.entities[i];
        RenderRequest &rr = registry.renderRequests.components[i];
        sorted_rr.emplace(entity, rr);
    }

    while (!sorted_rr.empty()) {
        Entity entity = sorted_rr.top().first;
        sorted_rr.pop();
        draw_list->push_back(entity);
    }
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw() {
    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(window, &w,
                           &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    gl_has_errors();

    // clear backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0.00001, 10);

    // Background colour
    glClearColor(50 / 255.f, 100 / 255.f, 225 / 255.f, 1.0f);

    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
                              // and alpha blending, one would have to sort
                              // sprites back to front
    gl_has_errors();

    // pos = projection * view * transform * vec;
    mat3 projection_2D = createProjectionMatrix();
    mat3 view_2D = createViewMatrix();

    // draw all entities with a render request to the frame buffer
    std::vector<Entity> draw_list = {};
    sortRenderRequests(&draw_list);

    for (Entity entity : draw_list) {
        // filter to entities that have a motion component
        if (registry.motions.has(entity)) {
            // Note, its not very efficient to access elements indirectly via the entity
            // albeit iterating through all Sprites in sequence. A good point to optimize
            drawTexturedMesh(entity, projection_2D, view_2D);
        } else if (registry.tilemaps.has(entity)) {
            drawTileMap(entity, projection_2D, view_2D);
        } else if (registry.text.has(entity)) {
            drawText(entity);
        }
    }

    drawParticles(projection_2D, view_2D);

    // draw framebuffer to screen
    drawToScreen();

    for (Entity entity : registry.overlays.entities) {
        drawTexturedMesh(entity, projection_2D, view_2D);
    }
}

void RenderSystem::renderGUI() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderSystem::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

mat3 RenderSystem::createProjectionMatrix() {
    // fake projection matrix, scaled to window coordinates
    float left = 0.f;
    float top = 0.f;
    float right = (float)WINDOW_WIDTH_PX;
    float bottom = (float)WINDOW_HEIGHT_PX;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);

    return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

mat3 RenderSystem::createViewMatrix() {
    Entity camera = registry.camera.entities[0];
    Motion &m = registry.motions.get(camera);

    return {{1.f, 0.f, 0.f},
            {0.f, 1.f, 0.f},
            {-(m.position.x - WINDOW_WIDTH_PX / 2.f), -(m.position.y - WINDOW_HEIGHT_PX / 2.f), 1.f}};
}

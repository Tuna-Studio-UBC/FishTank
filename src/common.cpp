#include "common.hpp"

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale) {
    mat3 S = {{scale.x, 0.f, 0.f}, {0.f, scale.y, 0.f}, {0.f, 0.f, 1.f}};
    mat = mat * S;
}

void Transform::rotate(float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat3 R = {{c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}};
    mat = mat * R;
}

void Transform::translate(vec2 offset) {
    mat3 T = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {offset.x, offset.y, 1.f}};
    mat = mat * T;
}

bool gl_has_errors() {
    GLenum error = glGetError();

    if (error == GL_NO_ERROR)
        return false;

    while (error != GL_NO_ERROR) {
        const char *error_str = "";
        switch (error) {
        case GL_INVALID_OPERATION:
            error_str = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_str = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }

        fprintf(stderr, "OpenGL: %s", error_str);
        error = glGetError();
        assert(false);
    }

    return true;
}

vec2 direction_to_unit_vector(float radians) {
    vec2 unit_vec = vec2(std::cos(radians), std::sin(radians));
    return unit_vec;
}

void change_vector_direction(vec2 &in_vec, float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);

    float x = in_vec.x * c - in_vec.y * s;
    float y = in_vec.x * s + in_vec.y * c;

    in_vec.x = x;
    in_vec.y = y;
}

vec2 get_vector_rotated(vec2 &in_vec, float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return {in_vec.x * c - in_vec.y * s, in_vec.x * s + in_vec.y * c};
}

bool entity_in_radius(vec2 dist, vec2 target_scale, float radius) {
    float true_radius = radius + std::max(target_scale.x, target_scale.y);

    float dist2 = dist.x * dist.x + dist.y * dist.y;

    return dist2 <= true_radius * true_radius;
}

ivec2 world_to_tile(const vec2 &world_pos) {
    int tx = static_cast<int>(world_pos.x / TILE_WIDTH_PX);
    int ty = static_cast<int>(world_pos.y / TILE_HEIGHT_PX);
    return {tx, ty};
}

vec2 tile_center_to_world(const ivec2 &tile) {
    float wx = tile.x * TILE_WIDTH_PX + (TILE_WIDTH_PX / 2);
    float wy = tile.y * TILE_HEIGHT_PX + (TILE_HEIGHT_PX / 2);
    return vec2(wx, wy);
}

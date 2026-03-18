#version 330

layout(location = 0) in vec2 in_position;        // quad vertex
layout(location = 1) in vec2 in_texcoord;        // quad uv
layout(location = 2) in vec2 instance_position;  // particle center

out vec2 texcoord;

uniform mat3 projection;
uniform mat3 view;
uniform float particle_scale;

void main()
{
    texcoord = in_texcoord;

    // First, scale the quad
    vec2 scaled = in_position * particle_scale;

    // Then, move quad to particle center
    vec2 world = scaled + instance_position;
    vec3 pos = projection * view * vec3(world, 1.0);
    gl_Position = vec4(pos, 1.0);
}

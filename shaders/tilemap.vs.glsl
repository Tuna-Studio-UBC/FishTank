#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform mat3 view;

// Tilemap Specific Data
uniform vec2 tileset_size;
uniform vec2 atlas_offset;
uniform float tile_size;

void main()
{
	vec2 texel = (in_texcoord * tile_size);
	vec2 uv_texcoord = clamp(texel, vec2(1.0), vec2(31.0)) / tileset_size / tile_size;
	texcoord = atlas_offset + uv_texcoord;
	vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
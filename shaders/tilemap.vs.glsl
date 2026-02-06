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

void main()
{
	texcoord = atlas_offset + (in_texcoord / tileset_size);
	vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
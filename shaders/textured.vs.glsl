#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
uniform vec2 uv_offset;
uniform vec2 uv_scale;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform mat3 view;

void main()
{
	texcoord = in_texcoord * uv_scale + uv_offset;
	vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}

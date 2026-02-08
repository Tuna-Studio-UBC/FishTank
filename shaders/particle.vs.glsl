#version 330

in vec3 in_position;

uniform mat3 projection;
uniform mat3 view;

void main()
{
    vec3 pos = projection * view * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
    gl_PointSize = 25.0;
}
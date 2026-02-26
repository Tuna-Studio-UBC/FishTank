#version 330

layout(location = 0) in vec4 vertex; // vec4.xyzw = packed vec2 pos (xy) + vec2 tex_coord (zw)
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 transform;

void main()
{
    gl_Position = projection * transform * vec4(vertex.xy, 0, 1.0);
    TexCoords = vertex.zw;
}

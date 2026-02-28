#version 330

layout(location = 0) out vec4 out_color;

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;

void main()
{
    out_color = texture(sampler0, texcoord);
}

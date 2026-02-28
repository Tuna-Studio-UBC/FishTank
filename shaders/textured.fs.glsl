#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec3 tintColour;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 textureColour = texture(sampler0, vec2(texcoord.x, texcoord.y));
	vec3 tinted = mix(fcolor, tintColour, textureColour.a);
	color = vec4(tinted, 1.0) * textureColour;
}

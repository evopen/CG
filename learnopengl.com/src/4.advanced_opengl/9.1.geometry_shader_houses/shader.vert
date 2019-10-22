#version 450

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

out VS_OUT {
	vec3 Color;
} vs_out;

void main()
{
	gl_Position = vec4(aPos, 0, 1.0);
	vs_out.Color = aColor;
}
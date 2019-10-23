#version 450

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

layout (binding = 0) uniform UniformBlock {
	vec2 offsets[100];
};

void main()
{
	vec2 offset = offsets[gl_InstanceID];
	gl_Position = vec4(aPos + offset, 0, 1.0);
	Color = aColor;
}
#version 450

in vec3 aPos;

layout (binding = 1) uniform UniformBufferObject {
	mat4 projection;
	mat4 view;
} ubo;

uniform mat4 model;

void main()
{
	gl_Position = ubo.projection * ubo.view * model * vec4(aPos, 1.0);
}
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform UniformBufferObject{
	mat4 projection;
	mat4 view;
	mat4 model;
};

void main()
{
    gl_Position = projection * view * model * vec4(inPos, 1.0);
	gl_PointSize = 2;
	outColor = inColor;
}

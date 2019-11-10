#version 450

layout (location = 0) in vec3 aPos;

layout (binding = 0) uniform UniformBufferObject{
	mat4 projection;
	mat4 view;
};

out vec3 TexCoord;

void main()
{
	TexCoord = aPos;
	gl_Position = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
}
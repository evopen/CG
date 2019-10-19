#version 450

layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoord;

void main()
{
	TexCoord = aPos;
	gl_Position = projection * view * vec4(aPos, 1.0);
}
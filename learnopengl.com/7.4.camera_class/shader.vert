#version 450

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 inTexCoord;
out vec3 color;
out vec2 textureCoord;
uniform float horizontal_offset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	textureCoord = inTexCoord;
}
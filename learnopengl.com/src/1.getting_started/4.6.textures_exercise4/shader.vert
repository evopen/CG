#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
out vec3 color;
out vec2 textureCoord;
uniform float horizontal_offset;

void main()
{
    gl_Position = vec4(aPos, 1.0);
	gl_Position.x = gl_Position.x + horizontal_offset;
	color = inColor;
	textureCoord = inTexCoord;
}
#version 330 core

layout (location = 0) in vec3 aPos;
uniform float horizontal_offset;

void main()
{
    gl_Position = vec4(aPos, 1.0);
	gl_Position.y = -gl_Position.y;
	gl_Position.x = gl_Position.x + horizontal_offset;
}
#version 450

layout (location = 0) in vec2 aPos;

void main()
{
	gl_Position = vec4(aPos, 0, 1);
	gl_PointSize = 10;
}
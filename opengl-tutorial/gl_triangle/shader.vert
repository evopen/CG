#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

out vec3 fragmentColor;


uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(inPos, 1.0);
	fragmentColor = inColor;
}
#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in ATTR {
	vec4 Normal;
} gs_in[];

layout (binding = 0) uniform UniformBufferObject{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

void GenerateLine(int index)
{
	gl_Position = gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = gl_in[index].gl_Position + gs_in[index].Normal * 0.1;
	EmitVertex();
	EndPrimitive();
}

void main()
{
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}
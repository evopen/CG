#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in VS_OUT {
	vec3 Color;
} gs_in[];

out vec3 Color;

void buildHouse(vec4 position)
{
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:bottom-left
    EmitVertex();   
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:top
	Color = vec3(1,1,1);
    EmitVertex();
	EndPrimitive();
}

void main()
{
	Color = gs_in[0].Color;
	buildHouse(gl_in[0].gl_Position);
}
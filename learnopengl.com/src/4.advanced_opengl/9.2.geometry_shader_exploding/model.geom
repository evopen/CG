#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float time;

in ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} gs_in[];

out ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} gs_out;

vec3 GetNormal() {
	vec3 a = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
	return normalize(cross(a,b));
}

vec4 explode(vec4 position, vec3 normal)
{
	float magnitude = 2.0;
	vec3 direction = magnitude * ((sin(time) + 1.0) / 2.0) * normal;
    return position + vec4(direction, 0);
}


void main()
{
	vec3 normal = GetNormal();
	
	gl_Position = explode(gl_in[0].gl_Position, normal);
	gs_out.TexCoord = gs_in[0].TexCoord;
	EmitVertex();
	gl_Position = explode(gl_in[1].gl_Position, normal);
	gs_out.TexCoord = gs_in[1].TexCoord;
	EmitVertex();
	gl_Position = explode(gl_in[2].gl_Position, normal);
	gs_out.TexCoord = gs_in[2].TexCoord;
	EmitVertex();
	EndPrimitive();
}
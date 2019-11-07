#version 450

out vec4 FragColor;

in ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} fs_in;


void main()
{
	FragColor = vec4(0, 1, 0, 1);
}
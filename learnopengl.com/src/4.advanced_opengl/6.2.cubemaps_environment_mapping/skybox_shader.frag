#version 450

in vec3 TexCoord;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
	FragColor = texture(skybox, TexCoord);
}
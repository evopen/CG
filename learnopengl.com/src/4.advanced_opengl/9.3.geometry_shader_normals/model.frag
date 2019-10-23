#version 450

in ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} fs_in;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;

	float shininess;
};

uniform Material material;

out vec4 FragColor;

void main()
{
	FragColor = vec4(texture(material.texture_diffuse1, fs_in.TexCoord));
}

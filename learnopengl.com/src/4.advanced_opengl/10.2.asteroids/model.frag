#version 450

out vec4 FragColor;

in ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} fs_in;

uniform vec3 cameraPos;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;

	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform samplerCube skybox;
uniform Material material;
uniform DirLight dirLight;

uniform int textureMode;


void main()
{
	FragColor = texture(material.texture_diffuse1, fs_in.TexCoord);
}
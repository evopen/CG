#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

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

uniform Material material;
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
	vec3 normal_norm = normalize(Normal);
	vec3 viewDir_norm = normalize(cameraPos - FragPos);

	vec3 result = CalcDirLight(dirLight, normal_norm, viewDir_norm);

	FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	light.direction = normalize(light.direction);

	float diff = max(dot(-light.direction, normal), 0);

	vec3 reflectDir = reflect(light.direction, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), 128);

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord));

	return ambient + diffuse + specular;
}

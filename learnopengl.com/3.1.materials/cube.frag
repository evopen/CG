#version 450

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 cameraPos;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
	vec3 norm = normalize(Normal);

	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	vec3 ambient = material.ambient * light.ambient;

	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(reflectDir, viewDir), 0), material.shininess);
	vec3 specular = material.specular * spec * light.specular;

	vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1);
}
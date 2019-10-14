#version 450

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 cameraPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;

	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define NR_POINT_LIGHTS 4

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  

void main()
{
	vec3 normal_norm = normalize(Normal);
	vec3 viewDir_norm = normalize(cameraPos - FragPos);

	vec3 result = CalcDirLight(dirLight, normal_norm, viewDir_norm);

	for(int i = 0; i < NR_POINT_LIGHTS; i++) {
		result += CalcPointLight(pointLights[i], normal_norm, FragPos, viewDir_norm);
	}

	FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	light.direction = normalize(light.direction);

	float diff = max(dot(-light.direction, normal), 0);

	vec3 reflectDir = reflect(light.direction, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(lightDir, normal), 0);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	float dist = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)) * attenuation;
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord)) * attenuation;
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord)) * attenuation;

	return (ambient + diffuse + specular);
}
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

struct SpotLight {
	vec3 position;
	vec3 direction;

	float innerCutoff;
	float outerCutoff;

	float constant;
	float linear;
	float quadratic;

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
uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  


void main()
{
	vec3 normal_norm = normalize(Normal);
	vec3 viewDir_norm = normalize(cameraPos - FragPos);

	vec3 result = vec3(0);
	result = CalcDirLight(dirLight, normal_norm, viewDir_norm);

	for(int i = 0; i < NR_POINT_LIGHTS; i++) {
		result += CalcPointLight(pointLights[i], normal_norm, FragPos, viewDir_norm);
	}
	result += CalcSpotLight(spotLight, normal_norm, FragPos, viewDir_norm);

	FragColor = vec4(result, 1.0);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	float dist = length(fragPos - light.position);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 lightDir = normalize(FragPos - light.position);
	float theta = dot(lightDir, normalize(light.direction));
	vec3 result;
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	if(theta > light.outerCutoff) {
		float epsilon = light.innerCutoff - light.outerCutoff;
		float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0 ,1.0);

		float diff = max(dot(-lightDir, normal), 0);
		vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoord));

		vec3 reflectDir = reflect(lightDir, normal);
		float spec = max(dot(reflectDir, viewDir), 0);
		vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));
		result = ambient + (diffuse + specular) * attenuation * intensity;
	} else {
		result = ambient;
	}
	
	return result;
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
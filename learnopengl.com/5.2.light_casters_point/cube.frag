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

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

uniform Material material;
uniform Light light;

void main()
{
	float lightDist = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * lightDist + light.quadratic * lightDist * lightDist);

	vec3 norm = normalize(Normal);

	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(reflectDir, viewDir), 0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	vec3 result = (ambient + diffuse + specular) * attenuation;
    FragColor = vec4(result, 1);
}
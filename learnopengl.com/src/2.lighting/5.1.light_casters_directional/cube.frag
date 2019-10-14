#version 450

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 cameraPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
	float shininess;
};

struct Light {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
	vec3 norm = normalize(Normal);

	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(reflectDir, viewDir), 0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

	vec3 emission = texture(material.emission, TexCoord).rgb;

	vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1);
}
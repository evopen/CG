#version 450

in vec3 FragPos;
in vec3 Normal;

uniform samplerCube skybox;
uniform vec3 cameraPos;

uniform bool refraction;

out vec4 FragColor;

void main()
{
	float ratio = 1.0 / 1.52;
	vec3 viewDir = normalize(FragPos - cameraPos);
	vec3 R;
	if(refraction) {
		R = refract(viewDir, normalize(Normal), ratio);
	} else {
		R = reflect(viewDir, normalize(Normal));
	}
	FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
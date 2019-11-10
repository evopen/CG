#version 450
#define M_PI 3.1415926535897932384626433832795

in vec3 TexCoord;

uniform samplerCube skybox;
uniform vec3 bhPos;
uniform vec3 cameraPos;
uniform vec3 cameraFront;
uniform float r_s_km;

out vec4 FragColor;

void distort();

void main()
{
	vec3 bhDir = bhPos - cameraPos;
	if(dot(bhDir, cameraFront) / (length(bhDir)) > 0) {
		distort();
	} else {
		FragColor = texture(skybox, TexCoord);
	}
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void distort() {
	vec3 bhDir = bhPos - cameraPos;
	vec3 refractV = -(cross(cross(bhDir, TexCoord), TexCoord));
	float cosTheta = dot(TexCoord, bhDir) / (length(TexCoord) * length(bhDir));
	float bhDist = length(bhDir);
	float TexLength = bhDist * cosTheta;
	float refractLength = sqrt(bhDist * bhDist - TexLength * TexLength);
	

	if(refractLength < r_s_km) {
		FragColor = vec4(0,0,0,1);
	} else {
		float bendDegree = 2 * r_s_km / bhDist;
		vec3 distortCoord = vec3(rotationMatrix(cross(bhDir, TexCoord), bendDegree) * vec4(TexCoord, 0.f));
		FragColor = texture(skybox, distortCoord);
	}
}
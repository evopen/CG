#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out ATTR {
	vec4 Normal;
} vs_out;


layout (binding = 0) uniform UniformBufferObject{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	mat4 normalMatrix = mat4(mat3(transpose(inverse(view * model))));
    vs_out.Normal = projection * normalMatrix * vec4(aNormal, 0.0);
}
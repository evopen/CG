#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out ATTR {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
} vs_out;


layout (binding = 0) uniform UniformBufferObject{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
	vs_out.FragPos = (model * vec4(aPos, 1.0)).xyz;
	vs_out.TexCoord = aTexCoord;
}
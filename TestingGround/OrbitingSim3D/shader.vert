#version 450
layout (location = 0) in vec2 aPos;

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
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
	gl_PointSize = 10;
}

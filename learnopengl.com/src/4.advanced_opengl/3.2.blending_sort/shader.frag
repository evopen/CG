#version 450

in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D myTexture;

void main() {
	vec4 texColor = texture(myTexture, TexCoord);
	FragColor = texColor;
}
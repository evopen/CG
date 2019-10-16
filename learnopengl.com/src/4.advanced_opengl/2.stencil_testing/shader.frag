#version 450

in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D myTexture;

void main() {
	FragColor = texture(myTexture, TexCoord);
}
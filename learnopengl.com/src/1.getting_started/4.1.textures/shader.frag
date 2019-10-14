#version 450
in vec3 color;
in vec2 textureCoord;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(uTexture, textureCoord) * vec4(color, 1.0);
}
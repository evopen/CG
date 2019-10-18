#version 450
in vec3 color;
in vec2 textureCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float visibility;

out vec4 FragColor;

void main()
{
    FragColor = mix(texture(texture1, textureCoord), texture(texture2, textureCoord), visibility);
}
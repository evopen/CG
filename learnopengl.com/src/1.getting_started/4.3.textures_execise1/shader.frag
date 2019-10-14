#version 450
in vec3 color;
in vec2 textureCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

out vec4 FragColor;

void main()
{
    FragColor = mix(texture(texture1, textureCoord), texture(texture2, vec2(1-textureCoord.x, textureCoord.y)),0.2);
}
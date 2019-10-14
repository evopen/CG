#version 330 core
out vec4 FragColor;

uniform float green;

void main()
{
    FragColor = vec4(0,green,0,1);
}
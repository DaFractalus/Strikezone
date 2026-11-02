#version 330 core

out vec4 FragColor;

in float ratio;

uniform vec4 color;

void main()
{
	FragColor = color;
	FragColor.a = min(1,(1 - abs(ratio * 2 -1)) * 5);
	if(FragColor.a <= 0) discard;
}

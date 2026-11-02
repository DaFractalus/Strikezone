#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 transform;

out vec2 texCoord;

void main()
{
	texCoord = aTexCoord;
	gl_Position = transform * vec4(aPos.x, aPos.y, 0.f, 1);
}
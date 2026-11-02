#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec4 scrDisplaySize;

void main()
{
	TexCoord = aTexCoord;
	vec4 sizes = scrDisplaySize;
	vec2 pos = aPos * (sizes.z > sizes.w ? vec2(1, (sizes.x * sizes.w) / (sizes.y * sizes.z)) : vec2((sizes.y * sizes.z) / (sizes.w * sizes.x), 1));
	gl_Position = vec4(pos.x, pos.y, 0, 1);
}
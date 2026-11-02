#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec4 col;
out vec2 TexCoords;
out vec4 Color;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(pos, 0, 1);
    TexCoords = tex;
    Color = col;
}  
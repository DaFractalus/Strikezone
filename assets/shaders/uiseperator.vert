#version 330 core

uniform float lineLength;
uniform mat4 transform;

out float ratio;

void main()
{
	ratio = gl_VertexID % 2;
	gl_Position = transform * vec4( (ratio - 0.5) * lineLength, 0, 0, 1);
}
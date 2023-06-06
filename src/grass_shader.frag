#version 430 core

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec3 frag_position;
layout (location = 2) out vec3 frag_color;

in vec3 f_position;
in vec3 f_normal;

uniform vec3 color;

void main()
{
	frag_normal = f_normal;
	frag_color = color;
	frag_position = f_position * 0.01;
}

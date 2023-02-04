#version 410 core

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec3 frag_position;
in vec3 f_position;
in vec3 f_color;
in vec3 f_normal;

void main()
{
	frag_normal = f_normal;
	frag_position = f_position * 0.01;
}

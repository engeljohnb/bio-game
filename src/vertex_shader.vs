#version 460 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;

void main()
{
	gl_Position = vec4(v_position, 1.0);
}

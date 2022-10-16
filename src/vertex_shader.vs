#version 460 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;

uniform mat4 local_space;
uniform mat4 world_space;

void main()
{
	gl_Position = local_space * vec4(v_position, 1.0);
}

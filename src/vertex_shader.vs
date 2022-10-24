#version 460 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;

out vec3 v_normal;
out vec3 frag_position;
uniform mat4 world_space;
uniform mat4 view_space;
uniform mat4 projection_space;

void main()
{
	mat3 normal_world_space = mat3(transpose(inverse(world_space)));
	gl_Position = projection_space * view_space * world_space * vec4(v_position, 1.0);
	v_normal = normalize(normal_world_space * normal);
	frag_position = vec3(world_space * vec4(v_position, 1.0));
	//frag_position = normal;
}

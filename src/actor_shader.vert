/*
    Bio-Game is a game for designing your own microorganism. 
    Copyright (C) 2022 John Engel 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/



#version 410 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;
layout (location = 3) in ivec4 bone_ids;
layout (location = 4) in vec4 bone_weights;

const int MAX_BONES = 25;
out vec3 f_normal;
out vec3 f_position;
uniform mat4 world_space;
uniform mat4 projection_view_space;
uniform mat4 bone_matrices[25];

void main()
{
	mat3 normal_world_space = mat3(transpose(inverse(world_space)));
	vec4 final_position = vec4(v_position, 1.0f);
	vec3 final_normal = vec3(normal);
	for (int i = 0; i < 4; ++i)
	{
		if ((bone_ids[i] < 0) || (bone_ids[i] >= 25))
		{
			continue;
		}
		vec4 local_position = bone_matrices[bone_ids[i]] * vec4(v_position, 1.0);
		final_position += local_position * bone_weights[i];
		vec3 local_normal = normal * mat3(bone_matrices[bone_ids[i]]) * bone_weights[i];
		final_normal += local_normal;
	}
	gl_Position = projection_view_space * world_space * final_position;
	f_normal = normalize(normal_world_space * final_normal);
	f_position = vec3(world_space * final_position);
}

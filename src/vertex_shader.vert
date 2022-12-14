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



#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;
layout (location = 3) in ivec4 bone_ids;
layout (location = 4) in vec4 bone_weights;

const int MAX_BONES = 25;
out vec3 v_normal;
out vec3 frag_position;
uniform mat4 local_space;
uniform mat4 world_space;
uniform mat4 view_space;
uniform mat4 projection_space;
//uniform mat4 bone_matrices[MAX_BONES];
uniform mat4 animation_transform;

void main()
{
	mat3 normal_world_space = mat3(transpose(inverse(world_space)));
	/*vec4 total_bone_influence = vec4(0.0);
	for (int i = 0; i < 4; ++i)
	{
		if (bone_ids[i] == -1)
		{
			continue;
		}
		vec4 bone_influence = bone_matrices[bone_ids[i]] * vec4(v_position, 1.0);
		total_bone_influence += bone_influence * bone_weights[i];
	}
	if (total_bone_influence == vec4(0.0))
	{
		total_bone_influence = vec4(v_position, 1.0);
	}
	total_bone_influence = vec4(v_position, 1.0);*/
	gl_Position = projection_space * view_space * world_space * animation_transform * vec4(v_position, 1.0);
	v_normal = normalize(normal_world_space * normal);
	frag_position = vec3(world_space * animation_transform * vec4(v_position, 1.0));
	//frag_position = normal;
}

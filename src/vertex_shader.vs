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

out vec3 v_normal;
out vec3 frag_position;
uniform mat4 local_space;
uniform mat4 world_space;
uniform mat4 view_space;
uniform mat4 projection_space;

void main()
{
	mat3 normal_world_space = mat3(transpose(inverse(world_space)));
	gl_Position = projection_space * view_space * world_space * local_space *vec4(v_position, 1.0);
	v_normal = normalize(normal_world_space * normal);
	frag_position = vec3(world_space * vec4(v_position, 1.0));
	//frag_position = normal;
}

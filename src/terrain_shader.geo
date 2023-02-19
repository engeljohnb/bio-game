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

#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

uniform mat4 projection_view_space;
out vec3 f_position;
out vec3 f_normal;
out vec2 f_offset;

void main()
{
	for (int i = 0; i < gl_in.length(); ++i)
	{
		vec4 pos = projection_view_space * gl_in[i].gl_Position; 
		gl_Position = pos;
		vec3 a = vec3(gl_in[0].gl_Position);
		vec3 b = vec3(gl_in[1].gl_Position);
		vec3 c = vec3(gl_in[2].gl_Position);
		f_normal = normalize(cross((b-a), (c-a)));
		f_position = vec3(gl_in[i].gl_Position);
		EmitVertex();
	}
	EndPrimitive();
}

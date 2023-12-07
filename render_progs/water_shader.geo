/* Bio-Game is a game for designing your own organism. 
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
uniform vec3 frustum_corners[8];
uniform int temperature;
uniform float precipitation;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_offset;
out vec2 f_tex_coords;

in ETESS_OUT
{
	float xz_scale;
	vec2 g_tex_coords;
} gs_in[];

vec3 get_frustum_normal(int i)
{
	switch (i)
	{
		case 0:
		{
			return normalize(cross(frustum_corners[1] - frustum_corners[0], frustum_corners[2] - frustum_corners[0]));
			break;
		}
		case 1:
		{
			return normalize(cross(frustum_corners[5] - frustum_corners[4], frustum_corners[1] - frustum_corners[4]));
			break;
		}
		case 2:
		{
			return normalize(cross(frustum_corners[2] - frustum_corners[3], frustum_corners[6] - frustum_corners[3]));
			break;
		}
		case 3:
		{
			return -normalize(cross(frustum_corners[5] - frustum_corners[7], frustum_corners[6] - frustum_corners[7]));
			break;
		}
	}
}

float get_d(int i)
{
	switch (i)
	{
		case 0:
		{
			return dot(frustum_corners[1], -get_frustum_normal(i));
			break;
		}
		case 1:
		{
			return dot(frustum_corners[1], -get_frustum_normal(i));
			break;
		}
		case 2:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
			break;
		}
		case 3:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
			break;
		}
	}
}

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[1].gl_Position);
	vec3 c = vec3(gl_in[2].gl_Position);
	f_normal = normalize(cross((b-a), (c-a)));

	for (int i = 0; i < gl_in.length(); ++i)
	{
		bool in_frustum = true;
		for (int j = 0; j < 6; ++j)
		{
			vec3 normal = get_frustum_normal(j);
			if ((dot(normal, vec3(gl_in[i].gl_Position)) + get_d(j)) < -70)
			{
				in_frustum = false;
				break;
			}
		}
		if (!in_frustum)
		{
			continue;
		}
		f_position = vec3(gl_in[i].gl_Position);
		vec4 pos = projection_view_space * gl_in[i].gl_Position; 
		f_tex_coords = gs_in[i].g_tex_coords;

		gl_Position = pos;
		EmitVertex();
	}

	EndPrimitive();
}

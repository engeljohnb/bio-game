/* Bio-Game is a game for designing your own microorganism. 
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
uniform int heightmap_width;
uniform int heightmap_height;
uniform float precipitation;
uniform sampler2D heightmap;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_offset;
out vec2 f_tex_coords;
out float f_snow_value;
out vec3 f_snow_normal;

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

#define PLUS_X 0
#define PLUS_Z 1
#define MINUS_X 2
#define MINUS_Z 3
#define PLUS_XZ 4
#define MINUS_XZ 5
#define PLUS_X_MINUS_Z 6
#define PLUS_Z_MINUS_X 7

vec3 get_snow_normal(vec3 position, vec2 g_tex_coords, float xz_scale)
{
	float delta_x = 1.0/heightmap_width;
	float delta_z = 1.0/heightmap_height;

	float offset_x = delta_x * xz_scale;
	float offset_z = delta_z * xz_scale;

	float height_dx = texture(heightmap, g_tex_coords + vec2(delta_x, 0.0)).r * texture(heightmap, g_tex_coords + vec2(delta_x, 0.0)).g;
	float height_dz = texture(heightmap, g_tex_coords + vec2(0.0, delta_z)).r * texture(heightmap, g_tex_coords + vec2(0.0, delta_z)).g;
	float height_dxdz = texture(heightmap, g_tex_coords + vec2(delta_x, delta_z)).r * texture(heightmap, g_tex_coords + vec2(delta_x, delta_z)).g;

	vec3 pos_dx = vec3(position.x + offset_x, height_dx, position.z);
	vec3 pos_dz = vec3(position.x, height_dz, position.z + offset_z);
	vec3 pos_dxdz = vec3(position.x+offset_x, height_dxdz, position.z + offset_z);

	return normalize(cross(pos_dz - pos_dx, pos_dxdz - pos_dx));
}

vec3 get_snow_border_normal(vec3 normal, vec2 g_tex_coords, float snow_value)
{
	float delta_x = 1.0/heightmap_width;
	float delta_z = 1.0/heightmap_height;
	vec3 xz_part = vec3(0.0);
	float max_difference= -1.0;
	int max_difference_index = -1;

	float differences[8];
	differences[PLUS_X] = snow_value - texture(heightmap, g_tex_coords + vec2(delta_x, 0.0)).b;
	differences[PLUS_Z] = snow_value - texture(heightmap, g_tex_coords + vec2(0.0, delta_z)).b;
	differences[MINUS_X] = snow_value - texture(heightmap, g_tex_coords + vec2(-delta_x, 0.0)).b;
	differences[MINUS_Z] = snow_value - texture(heightmap, g_tex_coords + vec2(0.0, -delta_z)).b;
	differences[PLUS_XZ] = snow_value - texture(heightmap, g_tex_coords + vec2(delta_x, delta_z)).b;
	differences[MINUS_XZ] = snow_value - texture(heightmap, g_tex_coords + vec2(-delta_x, -delta_z)).b;
	differences[PLUS_X_MINUS_Z] = snow_value - texture(heightmap, g_tex_coords + vec2(delta_x, -delta_z)).b;
	differences[PLUS_Z_MINUS_X] = snow_value - texture(heightmap, g_tex_coords + vec2(-delta_x, delta_z)).b;

	for (int i = 0; i < 8; ++i)
	{
		if (max_difference < differences[i])
		{
			max_difference = differences[i];
			max_difference_index = i;
		}
	}

	switch (max_difference_index)
	{
		case PLUS_X:
			xz_part = cross(normal, vec3(0.0, 0.0, 1.0));
			break;
		case PLUS_Z:
			xz_part = cross(normal, vec3(-1.0, 0.0, 0.0));
			break;
		case MINUS_X:
			xz_part = cross(normal, vec3(0.0, 0.0, -1.0));
			break;
		case MINUS_Z:
			xz_part = cross(normal, vec3(-1.0, 0.0, 0.0));
			break;
		case PLUS_XZ:
			xz_part = cross(normal, vec3(-1.0, 0.0, 1.0));
			break;
		case MINUS_XZ:
			xz_part = cross(normal, vec3(1.0, 0.0, -1.0));
			break;
		case PLUS_X_MINUS_Z:
			xz_part = cross(normal, vec3(1.0, 0.0, 1.0));
			break;
		case PLUS_Z_MINUS_X:
			xz_part = cross(normal, vec3(-1.0, 0.0, -1.0));
			break;
		default:
			return normal;
	}

	return normalize(mix(normalize(xz_part), normal, (1.0/(snow_value-0.35))));
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
			if ((dot(normal, vec3(gl_in[i].gl_Position)) + get_d(j)) < -30)
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
		f_snow_value = texture(heightmap, gs_in[i].g_tex_coords).b;
		f_snow_normal = f_normal;
		if ((f_snow_value >= 0.33) && (f_snow_value <= 0.36))
		{
			f_snow_normal = get_snow_border_normal(f_normal, f_tex_coords, f_snow_value);
		}
		else if (f_snow_value > 0.36)
		{
			f_snow_normal = get_snow_normal(vec3(gl_in[i].gl_Position), gs_in[i].g_tex_coords, gs_in[i].xz_scale);
		}

		gl_Position = pos;
		EmitVertex();
	}

	EndPrimitive();
}

/*
    Bio-Game is a game for designing your own organism. 
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
#define SHOW_LIGHTING 0
#define SHOW_POSITION 1
#define SHOW_NORMALS 2
#define SHOW_COLOR 3

struct PointLight
{
	vec3 	position;
	vec3	color;
	float	intensity;
};

struct DirectionLight
{
	vec3	direction;
	vec3	color;
	float	intensity;
};

in vec2 f_tex_coords;
out vec4 frag_color;
uniform sampler2D f_normal_texture;
uniform sampler2D f_position_texture;
uniform sampler2D f_color_texture;

uniform PointLight player_light;
uniform DirectionLight weather_light;
uniform DirectionLight tod_light;
uniform float view_distance;
uniform int mode;
uniform vec3 sky_color;
uniform vec3 camera_position;

vec4 calculate_direction_light(DirectionLight direction_light, vec3 position, vec3 normal)
{
	float angle = max(dot(direction_light.direction, normal), 0.0f); 
	vec4 return_vec = vec4(direction_light.color, 1.0);
	return_vec *= angle;
	return_vec *= direction_light.intensity;
	return return_vec;
}


vec4 calculate_point_light(PointLight point_light, vec3 position, vec3 normal)
{
	// All positions are scaled by 0.01 for the lighting stage, so 
	// the light's position should be too.
	vec3 light_position = point_light.position * 0.01f;
	vec3 direction = normalize(light_position - position);
	float angle = max(dot(direction, normal), 0.0f); 
	float dist = distance(light_position, position);
	float intensity = point_light.intensity / (dist*dist);
	vec4 return_vec = vec4(point_light.color, 1.0);
	return_vec *= angle;
	return_vec *= intensity;
	return return_vec;
}


void main()
{
	vec3 position = vec3(texture(f_position_texture, f_tex_coords));
	vec2 base_normal = vec2(texture(f_normal_texture, f_tex_coords).rg);
	vec3 normal = vec3(base_normal.r, base_normal.g, 1-(base_normal.r + base_normal.g));
	vec3 color = vec3(texture(f_color_texture, f_tex_coords));
	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	result += 0.2;
	result += calculate_point_light(player_light, position, normal);
	result += calculate_direction_light(weather_light, position, normal);
	result += calculate_direction_light(tod_light, position, normal);

	if (mode == SHOW_LIGHTING)
	{	if ((position == vec3(0.0f)) || (normal == vec3(0.0f)))
		{
			frag_color = vec4(sky_color, 1.0f);
		}
		else
		{
			float distance_from_camera = distance(camera_position, position);
			float percent_distance = distance_from_camera/view_distance;
			frag_color = mix(result * vec4(color, 1.0f), vec4(sky_color, 1.0f), clamp(pow(percent_distance, 2.0f)-0.25, 0.0f, 1.0));
		}
	}

	else if (mode == SHOW_POSITION)
	{ 
		frag_color = vec4(position, 1.0);
	}
	else if (mode == SHOW_NORMALS)
	{
		frag_color = vec4(normal, 1.0);
	}
	else if (mode == SHOW_COLOR)
	{
		frag_color = vec4(color, 1.0);
	}
}

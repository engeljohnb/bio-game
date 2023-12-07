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
#define SHOW_HEIGHT 4

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

uniform float sea_level;
uniform PointLight player_light;
uniform DirectionLight weather_light;
uniform DirectionLight tod_light;
uniform float view_distance;
uniform int mode;
uniform vec3 sky_color;
uniform vec3 camera_position;
uniform vec3 player_position;

uniform float rain_fog_percent;
uniform float dew_fog_percent;

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
	PointLight p_light = player_light;
	DirectionLight w_light = weather_light;
	DirectionLight t_light = tod_light;
	if (player_position.y < sea_level)
	{
		p_light.color.b += 0.4;
		w_light.color.b += 0.4;
		t_light.color.b += 0.4;

		p_light.color.r -= 0.3;
		w_light.color.r -= 0.3;
		t_light.color.r -= 0.3;
	}

	vec3 position = vec3(texture(f_position_texture, f_tex_coords));
	vec2 base_normal = vec2(texture(f_normal_texture, f_tex_coords).rg);
	vec3 normal = vec3(base_normal.r, base_normal.g, 1-(base_normal.r + base_normal.g));
	vec3 color = vec3(texture(f_color_texture, f_tex_coords));
	vec3 rain_color = vec3(0.15, 0.14, 0.26);
	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	bool rain_frag = false;

	if (color == vec3(1.0f))
	{
		rain_frag = true;
	}
	result += 0.25;
	result += calculate_point_light(p_light, position, normal);
	result += calculate_direction_light(w_light, position, normal);
	result += calculate_direction_light(t_light, position, normal);

	if (mode == SHOW_LIGHTING)
	{	
		if (rain_frag)
		{
			frag_color = vec4(rain_color, 1.0f);
		}
		else if ((position == vec3(0.0f)) || (normal == vec3(0.0f)))
		{
			frag_color = vec4(sky_color, 1.0f);
		}
		else
		{
			float distance_from_camera = distance(camera_position, position);
			float percent_distance = distance_from_camera/view_distance;
			//frag_color = mix(result * vec4(color, 1.0f), vec4(sky_color, 1.0f), clamp(pow(percent_distance, 2.0f)-0.25, 0.0f, 1.0));
			float fog_percent = clamp(min((1.0 - rain_fog_percent), (1.0 - dew_fog_percent)) * 10.0, 1.7, 10.0);
			//float fog_percent = max(rain_fog_percent, dew_fog_percent) * 10.0;
			frag_color = mix(result * vec4(color, 1.0f), vec4(sky_color, 1.0f), clamp(pow(percent_distance, fog_percent)-0.25, 0.0f, 1.0));
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
	else if (mode == SHOW_HEIGHT)
	{
		frag_color = vec4(vec3(position.y, position.y, position.y)/75.0,1.0);
	}
	else if (mode == SHOW_COLOR)
	{
		frag_color = vec4(color, 1.0);
	}
}

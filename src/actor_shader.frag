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
#define MAX_LIGHTS 1

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

in vec3 v_normal;
in vec3 frag_position;
uniform vec4 color;
uniform PointLight point_lights[MAX_LIGHTS];

out vec4 frag_color;

vec4 calculate_point_light(PointLight point_light)
{
	vec3 direction = normalize(point_light.position - frag_position);
	float angle = max(dot(direction, v_normal), 0.0f); 
	return vec4(point_light.color * point_light.intensity * angle, 1.0f);
	
}

void main()
{
	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		result += calculate_point_light(point_lights[i]);
	}
	frag_color = result*color;
	//frag_color = vec4(frag_position, 1.0);
	//frag_color = vec4( 0.5*(frag_position + vec3(1.0, 1.0, 1.0)), 1 );

	
}

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

in vec2 f_tex_coords;
out vec4 frag_color;
uniform sampler2D f_normal_texture;
uniform sampler2D f_position_texture;

struct PointLight
{
	vec3 	position;
	vec3	color;
	float	intensity;
};


vec4 calculate_point_light(PointLight point_light)
{
	vec3 f_position = vec3(texture(f_position_texture, f_tex_coords));
	vec3 f_normal = vec3(texture(f_normal_texture, f_tex_coords));
	vec3 direction = normalize(point_light.position - f_position);
	float angle = max(dot(direction, f_normal), 0.0f); 
	float dist = distance(point_light.position, f_position);
	float intensity = point_light.intensity / (dist*dist);
	return vec4(point_light.color * intensity * angle, 1.0f);
}

void main()
{
	PointLight point_light;
	point_light.position = vec3(4.0f, 4.0f, 0.0f);
	point_light.color = vec3(1.0f, 1.0f, 1.0f);
	point_light.intensity = 10.0f;

	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	result += calculate_point_light(point_light);
	frag_color = result * vec4(1.0, 0.0, 0.0, 1.0);
}

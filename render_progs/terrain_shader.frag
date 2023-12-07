#version 430 core
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

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec3 frag_position;
layout (location = 2) out vec3 frag_color;

in vec3 f_position;
in vec3 f_color;
in vec3 f_normal;
in vec2 f_tex_coords;
in float f_snow_value;
in vec3 f_snow_normal;

//uniform vec3 interpolation_samples[4];
uniform int temperature;
uniform float precipitation;
uniform float sea_level;


bool float_close_enough(float a, float b)
{
	return ((max(a, b) - min(a, b)) < 1.0);
}
bool vec3_equal(vec3 a, vec3 b)
{
	return ((float_close_enough(a.x, b.x)) &&
		(float_close_enough(a.z, b.z)));
}

void main()
{
	frag_normal = f_normal;
	frag_position = (f_position * 0.01f);
	vec3 base_color = vec3(0.16f, 0.19f, 0.061f);
	vec3 cold_no_snow = vec3(0.07f, 0.15f, 0.17f);
	vec3 desert_color = vec3(0.40f, 0.28f, 0.14f);
	vec3 snow_color = vec3(0.15f, 0.15f, 0.19f);
	vec3 underwater_color = vec3(0.07, 0.15, 0.25);
	frag_color = base_color;

	if (temperature < 45)
	{
		frag_color = mix(cold_no_snow, base_color, float(temperature)/140.0f);
	}
	if ((precipitation < 0.2f))
	{
		frag_color = mix(desert_color, base_color, 0.2-precipitation);
	}
	if ((f_position.y < sea_level) && (precipitation >= 0.2))
	{
		frag_color = vec3(0.07, 0.15, 0.25);
	}
	else if (f_snow_value >= 0.35f)
	{
		if (f_snow_normal != vec3(0.0f))
		{
			frag_normal = f_snow_normal;
		}
		frag_color = snow_color;
	}

	/*for (int i = 0; i < 4; ++i)
	{
		if (vec3_equal(f_position, interpolation_samples[i]))
		{
			frag_color = vec3(1.0, 0.0, 0.0);
		}
	}*/

//	frag_color = f_color;
}

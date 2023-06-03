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

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec3 frag_position;
layout (location = 2) out vec3 frag_color;

in vec3 f_position;
in vec3 f_color;
in vec3 f_normal;
in vec2 f_tex_coords;
in float f_snow_value;

uniform int temperature;
uniform float precipitation;

#define NOISE fbm
#define BITMAP_WIDTH 1024
#define BITMAP_HEIGHT 1024
#define NUM_NOISE_OCTAVES 5
#define MAX_TERRAIN_BLOCKS 100000

#define NUM_OCTAVES 5

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float fbm(vec2 x) {
	float v = 0.0;
	float a = 0.5;
	vec2 shift = vec2(100);
	// Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

//
// psrdnoise2.glsl
//
// Authors: Stefan Gustavson (stefan.gustavson@gmail.com)
// and Ian McEwan (ijm567@gmail.com)
// Version 2021-12-02, published under the MIT license (see below)
//
// Copyright (c) 2021 Stefan Gustavson and Ian McEwan.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,A


void main()
{
	frag_normal = f_normal;
	frag_position = (f_position * 0.01);

	vec3 base_color = vec3(0.4, 1.0, 0.6);
	vec3 cold_no_snow = vec3(0.0f, 0.64f, 0.74f);
	vec3 desert_color = vec3(0.3, 0.5, 0.44);
	vec3 snow_color = vec3(0.2, 0.7, 1.0);

	frag_color = base_color;
	if ((precipitation < 0.2))
	{
		frag_color = mix(desert_color, base_color, 0.2-precipitation);
	}
	else if (temperature < 45)
	{
		float p = float(temperature-32)/13;
		frag_color = mix(cold_no_snow, base_color, p);
	}

	if (f_snow_value > 0.0)
	{
		frag_color = mix(frag_color, vec3(1.0, 1.0, 1.0), f_snow_value);
	}
}

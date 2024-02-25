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
#define MAX_TERRAIN_BLOCKS 100000

layout (location = 0) in vec3 v_position;
uniform uint total;
uniform float scale_factor;
uniform uint terrain_index;
uniform int num_subgroups;
uniform vec3 base_position;
uniform float patch_size;

out VS_OUT
{
	vec3 	g_base_position;
	vec3 	g_position;
} vs_out;


mat4 translate(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0));
}

mat4 rotate(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

mat4 scale(vec3 axis)
{
	return mat4(
		vec4(axis.x, 0.0, 0.0, 0.0),
		vec4(0.0, axis.y, 0.0, 0.0),
		vec4(0.0, 0.0, axis.z, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0));
}

#define NOISE fbm
#define NUM_NOISE_OCTAVES 5
#define MAX_TERRAIN_BLOCKS 100000

#define NUM_OCTAVES 5

float rand(vec2 n) 
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main()
{
	float x_id = float(gl_InstanceID % total);
	float z_id = float(gl_InstanceID / total);

	int block_x = int((terrain_index % MAX_TERRAIN_BLOCKS) & 0xff);
	int block_z = int((terrain_index / MAX_TERRAIN_BLOCKS) & 0xff);
	int block = (block_x + block_z);

	int sub_id = gl_InstanceID % (block/20);
	float sub_x_id = float(sub_id % total);
	float sub_z_id = float(sub_id / total);

	float rand_num0 = rand(vec2(x_id, z_id));
	float rand_num1 = rand(vec2(z_id, x_id));
	float rand_num_sub0 = rand(vec2(sub_x_id, sub_z_id));
	float rand_num_sub1 = rand(vec2(sub_z_id, sub_x_id));

	float sub_coefficient = float(block)*2.0f;
	float individual_coefficient = float(block)/2.0f;

	vec3 subgroup_offset = (rotate(vec3(rand_num_sub0, 1.0, rand_num_sub1), 10.0/rand_num_sub0) * normalize(vec4(rand_num_sub0, rand_num_sub1, rand_num_sub0*rand_num_sub1, 1.0))).xyz * sub_coefficient;
	vec3 individual_offset = (rotate(vec3(rand_num0, 1.0, rand_num1), 10000.0/rand_num0) * normalize(vec4(rand_num0, rand_num1, rand_num0*rand_num1, 1.0))).xyz * individual_coefficient;

	mat4 scale = scale(vec3(1.5+(rand_num_sub0*5.0)));

	vec3 final_position = base_position + subgroup_offset + individual_offset;
	vs_out.g_position = final_position;
	vs_out.g_base_position = base_position;
	gl_Position = scale * vec4(v_position, 1.0);
}

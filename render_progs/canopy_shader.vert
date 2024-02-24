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

layout (location = 0) in vec3 v_position;
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

void main()
{
	int x_index = gl_InstanceID % int(patch_size);
	int z_index = gl_InstanceID / int(patch_size);
	float rand_num0 = fract(1000000*sin(gl_InstanceID));
	float rand_num1 = fract(1000000*cos(gl_InstanceID));
	float rand_num2 = fract(10000*sin(rand_num0*rand_num1/2.0));
	float patch_size_factor = 0.4;

	float coefficient = 150.0;
	float sub_coefficient = 20.0;

	float offx = rand_num0 - 0.5;
	float offy = rand_num1 - 0.5;
	float offz = rand_num2 - 0.5;

	float subgroup_randnum = fract(10000*sin(gl_InstanceID/num_subgroups));
	vec3 subgroup_base_offset = vec3(offx+subgroup_randnum, offy+subgroup_randnum, offz+subgroup_randnum) * coefficient;

	float _offx = offx * sub_coefficient;
	float _offy = offx * sub_coefficient;
	float _offz = offx * sub_coefficient;

	vec3 final_position = subgroup_base_offset + vec3(base_position.x + _offx, base_position.y + _offy, base_position.z + _offz);

	mat4 rotation = rotate(vec3(offx, 1, offz), rand_num0);
	mat4 displacement = mat4(1.0);
	mat4 recenter = translate(vec3(-0.5, -0.5, -0.5));
	mat4 inv_recenter = inverse(recenter);

	vs_out.g_position = final_position;
	vs_out.g_base_position = base_position;
	gl_Position = rotation * displacement * vec4(v_position, 1.0);
}

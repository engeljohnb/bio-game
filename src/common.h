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

#ifndef __COMMON_H__
#define __COMMON_H__
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "utils.h"
typedef unsigned int B_Shader;
typedef unsigned int B_Framebuffer;
typedef unsigned int B_Texture;

typedef struct 
{
	vec3 	position;
	vec3 	color;
	float 	intensity;
} PointLight;


B_Shader B_compile_simple_shader(const char *vert_path, const char *frag_path);
B_Shader B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *geo_path, const char *ctess_path, const char *etess_path);
B_Shader B_compile_compute_shader(const char *comp_path);
void B_free_shader(B_Shader shader);
int B_check_shader(unsigned int id, const char *name, int status);
void B_set_uniform_float(B_Shader shader, char *name, float value);
void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value);
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value);
void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value);
void B_set_uniform_int(B_Shader shader, char *name, int value);
void B_set_uniform_point_light(B_Shader shader, char *name, PointLight value);
B_Shader B_compile_grass_shader(const char *vert_path, const char *geo_path, const char *frag_path);
#endif

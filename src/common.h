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

#ifndef __COMMON_H__
#define __COMMON_H__
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "utils.h"

// Compilation flags
#define DRAW_DEBUG 0
#define USE_ALT_CAMERA 0
#define BENCHMARK 0
#define TERRAIN_XZ_SCALE 300

/* NOTE TO STRANGERS: The worlds are generated differently on different machines. These shortcuts are for me
 * during development, but won't work on your machine. Sorry :\ */
//#define PLAYER_TERRAIN_INDEX_START ((uint64_t)9200029000)

/* Center of map */
//#define PLAYER_TERRAIN_INDEX_START (MAX_TERRAIN_BLOCKS/4 * (MAX_TERRAIN_BLOCKS/2)) - (MAX_TERRAIN_BLOCKS/2)

/* Super duper grassy area */
#define PLAYER_TERRAIN_INDEX_START ((uint64_t)1745060245)

/* Warm/cold area border */
//#define PLAYER_TERRAIN_INDEX_START ((uint64_t)506192528)

/* Desert */
//#define PLAYER_TERRAIN_INDEX_START ((uint64_t)1649960493)

/* Weird floating ice */
//#define PLAYER_TERRAIN_INDEX_START ((uint64_t)1032650346)

/* Snowy area */
//#define PLAYER_TERRAIN_INDEX_START 1250550005 

/* Warm area */
//#define PLAYER_TERRAIN_INDEX_START 1249349995
//#define PLAYER_TERRAIN_INDEX_START 302093068

#define SEA_LEVEL 100

/* MAX_TERRAIN_BLOCKS is not the total maximum number of terrain blocks, but rather the 
 * total number of terrain blocks in either the x or z direction. So the total number
 * of terrain blocks would be MAX_TERRAIN_BLOCKS * MAX_TERRAIN_BLOCKS. */
#define MAX_TERRAIN_BLOCKS 100000

typedef unsigned int B_Shader;
typedef unsigned int B_Framebuffer;
typedef unsigned int B_Texture;

typedef struct PointLight
{
	vec3 	position;
	vec3 	color;
	float 	intensity;
} PointLight;

typedef struct DirectionLight
{
	vec3 	direction;
	vec3	color;
	float	intensity;
} DirectionLight;

// TODO: turn this back into a constant
void set_view_distance(float distance);
float get_view_distance(void);
DirectionLight create_direction_light(vec3 direction, vec3 color, float intensity);
B_Shader B_compile_simple_shader(const char *vert_path, const char *frag_path);
B_Shader B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *geo_path, const char *ctess_path, const char *etess_path);
B_Shader B_compile_compute_shader(const char *comp_path);
void B_free_shader(B_Shader shader);
int B_check_shader(unsigned int id, const char *name, int status);
void B_set_uniform_float(B_Shader shader, char *name, float value);
void B_set_uniform_vec2(B_Shader shader, char *name, vec2 value);
void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value);
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value);
void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value);
void B_set_uniform_int(B_Shader shader, char *name, int value);
void B_set_uniform_point_light(B_Shader shader, char *name, PointLight value);
void B_set_uniform_direction_light(B_Shader shader, char *name, DirectionLight value);
B_Shader B_compile_simple_shader_with_geo(const char *vert_path, const char *geo_path, const char *frag_path);
int should_print_debug(void);
void set_should_print_debug(int i);
#endif

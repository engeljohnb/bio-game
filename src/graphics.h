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

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__
#define MAX_MESHES 16
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "window.h"

#define TRIANGLE_SIZE sizeof(float)*27

typedef unsigned int B_Shader;
typedef struct
{
	vec3	position;
	vec3	normal;
	vec3	tex_coords;	
} B_Vertex;

typedef struct
{
	B_Vertex 	*vertices;
	int		active;
	int 		num_vertices;
	unsigned int 	vao;
	unsigned int	vbo;
	unsigned int	ebo;
	
} B_Mesh;

typedef struct
{
	mat4	local_space;
	mat4	world_space;
	//mat4	view;  << This is the camear??
	B_Mesh 	meshes[MAX_MESHES];
} B_Model;


B_Model B_create_triangle();
B_Model B_create_model(B_Vertex *vertices, unsigned int num_vertices);
void get_triangle_data(B_Vertex *buffer);
B_Model B_create_cube();
void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value);
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value);
void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value);
void B_blit_model(B_Model model, B_Shader shader);
void B_free_model(B_Model model);

#endif

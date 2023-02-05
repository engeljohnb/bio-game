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

#ifndef __TERRAIN_H__
#define __TERRAINN_H__
#include <glad/glad.h>
#include "rendering.h"
#include "camera.h"

typedef struct
{
	unsigned int	vao;
	unsigned int	vbo;
	unsigned int	g_buffer;
	unsigned int	lighting_vao;
	unsigned int	lighting_vbo;
	int		num_vertices;
	int		num_columns;
	unsigned int	normal_texture;
	unsigned int	position_texture;
} TerrainMesh;

typedef struct
{
	GLfloat		position[3];
	GLfloat		tex_coords[2];
} T_Vertex;

T_Vertex *generate_t_vertices(int width, int height);
TerrainMesh B_send_terrain_mesh_to_gpu(B_Framebuffer g_buffer, T_Vertex *vertices, int num_vertices, int num_columns);
void B_draw_terrain(TerrainMesh mesh, B_Shader shader, Camera *camera);
TerrainMesh B_create_terrain_mesh(B_Framebuffer g_buffer, int width, int height);
#endif

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


/* MAX_TERRAIN_BLOCKS is not the total maximum number of terrain blocks, but rather the 
 * total number of terrain blocks in either the x or z direction. So the total number
 * of terrain blocks would be MAX_TERRAIN_BLOCKS * MAX_TERRAIN_BLOCKS. */
#define MAX_TERRAIN_BLOCKS 100000
#define TERRAIN_SCALE 300

typedef struct
{
	unsigned int	vao;
	unsigned int	vbo;
	unsigned int	g_buffer;
	int		num_vertices;
	int		num_columns;
} TerrainMesh;

/* A TerrainBlock is a block of nine 4*SCALE x 4*SCALE terrain_meshes. You could think of them as like a tile map. */
typedef struct
{
	TerrainMesh	terrain_meshes[9];
	/* The index of the block that the player is currently over */
	int		current_block_index;
} TerrainBlock;

typedef struct
{
	GLfloat		position[3];
	GLfloat		tex_coords[2];
} T_Vertex;

T_Vertex *generate_t_vertices(int width, int height);
TerrainMesh B_send_terrain_mesh_to_gpu(B_Framebuffer g_buffer, T_Vertex *vertices, int num_vertices, int num_columns);
void B_draw_terrain_mesh(TerrainMesh mesh, B_Shader shader, Camera *camera, int my_block_index, int player_block_index, float tessellation_level);

/* Draws the terrain meshes that are currently surrounding the player (* is the player's current tile).
 * The geography is generated dynamically -- so only one TerrainBlock is needed. It's simply repositioned and
 * the height recalculated based on the player's location.
 * |-------|-------|-------|
 * |       |       |       |
 * |       |       |       |
 * |       |       |       |
 * |-------|-------|-------|
 * |       |       |       |
 * |       |   *   |       |
 * |       |       |       |
 * |-------|-------|-------|
 * |       |       |       |
 * |       |       |       |
 * |       |       |       |
 * |-------|-------|-------|
 * */
void draw_terrain_block(TerrainBlock *block, B_Shader shader, Camera *camera, int terrain_block_index);
TerrainBlock create_terrain_block(B_Framebuffer g_buffer);
TerrainMesh B_create_terrain_mesh(B_Framebuffer g_buffer);
void free_terrain_block(TerrainBlock *block);
#endif

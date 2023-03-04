/*
    Bio-Game is a game for designing your own microorganism.  Copyright (C) 2022 John Engel 

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
#define __TERRAIN_H__
#include <glad/glad.h>
#include "common.h"
/* MAX_TERRAIN_BLOCKS is not the total maximum number of terrain blocks, but rather the 
 * total number of terrain blocks in either the x or z direction. So the total number
 * of terrain blocks would be MAX_TERRAIN_BLOCKS * MAX_TERRAIN_BLOCKS. */
#define MAX_TERRAIN_BLOCKS 100000
#define TERRAIN_HEIGHT_SCALE 800
#define TERRAIN_XZ_SCALE 300
typedef struct
{
	unsigned int	vao;
	unsigned int	vbo;
	unsigned int	g_buffer;
	int		num_vertices;
	int		num_columns;
} TerrainMesh;

/* A TerrainBlock is a block of nine 4*TERRAIN_XZ_SCALE x 4*TERRAIN_XZ_SCALE terrain_meshes. You could think of them as like a tile map. */
//TODO: If I'm going to keep calling one of the individual meshes a "block", I shouldn't also call the collection
//of all of them a "block". Maybe call it a TerrainChunk?
typedef struct
{
	float		*heightmap_buffer;
	float		tessellation_level;
	int		heightmap_width;
	int		heightmap_height;
	int		block_width;
	int		block_height;
	size_t		heightmap_size;
	TerrainMesh	terrain_meshes[9];
	unsigned int 	heightmap_texture;
	unsigned int	g_buffer;
	unsigned int	compute_shader;
	float		*tex_coords[2];
} TerrainBlock;

typedef struct
{
	GLfloat		position[3];
	GLfloat		tex_coords[2];
} T_Vertex;

T_Vertex *generate_t_vertices(int width, int height);
TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int  g_buffer, T_Vertex *vertices, int num_vertices, int num_columns);

/* Draws the terrain meshes that are currently surrounding the player (* marks the player's current tile).
 * The geography is generated dynamically -- so only one TerrainBlock is needed throughout the game. It's simply repositioned and
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
TerrainBlock create_terrain_block(unsigned int g_buffer);
TerrainBlock create_server_terrain_block(void);
TerrainMesh B_create_terrain_mesh(unsigned int g_buffer);
void free_terrain_block(TerrainBlock *block);
void B_free_terrain_mesh(TerrainMesh mesh);
void B_send_terrain_block_to_gpu(TerrainBlock *block);
void B_update_terrain_block(TerrainBlock *block, int player_block_index);
unsigned int B_compile_compute_shader(const char *comp_path);
void draw_terrain_block(TerrainBlock *block, B_Shader shader, mat4 projection_view, int player_block_index);
void B_draw_terrain_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			int my_block_index, 
			int player_block_index, 
			float tessellation_level, 
			B_Texture texture);


#endif

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
	unsigned int 	vao;
	unsigned int	vbo;
	unsigned int	ebo;
	unsigned int	instance_vbo;
	unsigned int	g_buffer;
	B_Shader	shader;
	int		num_vertices;
	int		num_elements;
	B_Texture	heightmap_texture;
	B_Texture	density_texture;
	int		density_texture_width;
	int		density_texture_height;
} TerrainElementMesh;

typedef struct
{
	unsigned int	vao;
	unsigned int	vbo;
	unsigned int	g_buffer;
	int		num_vertices;
	int		num_rows;
} TerrainMesh;

typedef struct
{
	float		value;
	float		scale;
} TerrainHeight;

/* A TerrainChunk is a block of nine 4*TERRAIN_XZ_SCALE x 4*TERRAIN_XZ_SCALE terrain_meshes. You could think of them as like a tile map. */
typedef struct
{
	TerrainHeight	*heightmap_buffer;
	float		tessellation_level;
	int		heightmap_width;
	int		heightmap_height;
	int		block_width;
	int		block_height;
	unsigned int	heightmap_size;
	TerrainMesh	terrain_meshes[9];
	B_Texture 	heightmap_texture;
	B_Framebuffer	g_buffer;
	B_Shader 	compute_shader;
	float		*tex_coords[2];
} TerrainChunk;

typedef struct
{
	GLfloat		position[3];
	GLfloat		tex_coords[2];
} T_Vertex;

T_Vertex *generate_t_vertices(int width, int height);
TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int  g_buffer, T_Vertex *vertices, int num_vertices, int num_rows);

/* Draws the terrain meshes that are currently surrounding the player (* marks the player's current tile).
 * The geography is generated dynamically -- so only one TerrainChunk is needed throughout the game. It's simply repositioned and
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
TerrainChunk create_terrain_chunk(unsigned int g_buffer);
TerrainChunk create_server_terrain_chunk(void);
TerrainMesh B_create_terrain_mesh(unsigned int g_buffer);
void free_terrain_chunk(TerrainChunk *block);
void B_free_terrain_mesh(TerrainMesh mesh);
void B_send_terrain_chunk_to_gpu(TerrainChunk *block);
void B_update_terrain_chunk(TerrainChunk *block, int player_block_index);
unsigned int B_compile_compute_shader(const char *comp_path);
void draw_terrain_chunk(TerrainChunk *block, B_Shader shader, mat4 projection_view, int player_block_index);
void B_draw_terrain_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			int my_block_index, 
			int player_block_index, 
			float tessellation_level, 
			B_Texture texture);
TerrainElementMesh create_grass_blade(int g_buffer, B_Texture heightmap_texture);
void B_free_terrain_element_mesh(TerrainElementMesh mesh);
void B_draw_terrain_element_mesh(TerrainElementMesh mesh, mat4 projection_view, vec3 player_position);

#endif

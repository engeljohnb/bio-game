/*
    Bio-Game is a game for designing your own organism.  Copyright (C) 2022 John Engel 

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

#define TERRAIN_HEIGHT_FACTOR 2500

enum TERRAIN_CHUNK_TYPES
{
	TERRAIN_CHUNK_WATER,
	TERRAIN_CHUNK_LAND,
};

typedef struct TerrainHeight
{
	float		value;
	float		scale;
	float		snow;
	float		padding;
} TerrainHeight;

typedef struct TerrainMesh
{
	unsigned int	vao;
	unsigned int	vbo;
	unsigned int	ebo;
	unsigned int	g_buffer;
	unsigned int	*faces;
	int		use_ebo;
	int		num_faces;
	int		num_vertices;
	int		num_rows;
} TerrainMesh;

typedef struct TerrainElementMesh
{
	unsigned int		vao;
	unsigned int		vbo;
	unsigned int		ebo;
	unsigned int		num_elements;
	unsigned int		num_vertices;
	B_Framebuffer		g_buffer;
	B_Texture		heightmap;
	B_Shader		shader;
} TerrainElementMesh;

typedef struct Plant
{	
	TerrainElementMesh 	mesh;
	int 			min_temperature;
	int			max_temperature;
	int			ideal_min_temperature;
	int			ideal_max_temperature;
	float 			min_precipitation;
	float 			max_precipitation;

} Plant;

/* A TerrainChunk is a block of nine 4*get_terrain_xz_scale() x 4*get_terrain_xz_scale() terrain_meshes. You could think of them as like a tile map.
 * Whenever a "block" is referred to in the code, it's usally indicating one of these nine meshes, and a "chunk" usually indicates
 * all nine of them together. */
typedef struct TerrainChunk
{
	int		type;
	TerrainHeight	*heightmap_buffer;
	float		tessellation_level;
	int		heightmap_width;
	int		heightmap_height;
	int		width;
	int		height;
	/* dimension is the width and breadth of the TerrainChunk in terrain_meshes. */
	int		dimension;
	unsigned int	heightmap_size;
	TerrainMesh	terrain_mesh;
	B_Texture 	heightmap;
	B_Texture	snow_normal_map;
	B_Framebuffer	g_buffer;
	B_Shader 	compute_shader;
	B_Shader 	vertex_compute_shader;
	//float		*tex_coords[2];
} TerrainChunk;

typedef struct T_Vertex
{
	GLfloat		position[3];
} T_Vertex;

typedef struct TerrainVertexData
{
	T_Vertex	*vertices;
	unsigned int	*faces;
	unsigned int	num_vertices;
	unsigned int	num_faces;
} TerrainVertexData;

T_Vertex *generate_t_vertices(int width, int height);
TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int  g_buffer, T_Vertex *vertices, int num_vertices, int num_rows);
void B_send_terrain_mesh_to_gpu_ebo(TerrainMesh *mesh, TerrainVertexData *vertex_data);

/* Draws the terrain meshes that are currently surrounding the player (* marks the player's current tile).
 * The geography is generated dynamically -- so only one TerrainChunk is needed throughout the game. It's simply repositioned and
 * the heightmap recalculated based on the player's location.
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
TerrainChunk create_terrain_chunk(unsigned int g_buffer, int type, unsigned long terrain_index);
TerrainChunk create_server_terrain_chunk(void);
TerrainMesh B_create_terrain_mesh(unsigned int g_buffer);
void free_terrain_chunk(TerrainChunk *block);
void B_free_terrain_mesh(TerrainMesh mesh);
void B_send_terrain_chunk_to_gpu(TerrainChunk *block);
void B_update_terrain_chunk(TerrainChunk *block, uint64_t player_block_index);
unsigned int B_compile_compute_shader(const char *comp_path);
void draw_land_terrain_chunk(TerrainChunk *block, B_Shader shader, mat4 projection_view, uint64_t player_block_index, vec3 player_facing);
void draw_water_terrain_chunk(TerrainChunk *block, B_Texture land_heightmap, B_Shader shader, mat4 projection_view, uint64_t player_block_index, vec3 player_facing);
void draw_land_terrain_chunk_debug(TerrainChunk *chunk, 
				   B_Shader shader, 
				   mat4 projection_view, 
				   uint64_t player_block_index, 
				   vec3 player_facing,
				   vec3 grass_patch_centers[9],
				   float grass_patch_max_distance);

/* sets the terrain_chunk's dimension (the width and breadth of the terrain_chunk in terrain_meshes).
 * If it's even, it wil be rounded up to the next odd number. It makes the math a little easier if
 * the terrain chunk has a center tile. */
int set_terrain_chunk_dimension(int dimension);
int get_terrain_chunk_dimension(void);
void get_terrain_heightmap_size(int *w, int *h);
TerrainMesh load_terrain_mesh_from_file(B_Framebuffer g_buffer, const char *filename);
#endif

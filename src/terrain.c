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

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <math.h>
#include <string.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include "camera.h"
#include "asset_loading.h"
#include "environment.h"
#include "time.h"
#include "noise.h"
#include "utils.h"
#include "input.h"
#include "terrain.h"
#include "debug.h"

int g_terrain_heightmap_width;
int g_terrain_heightmap_height;
int g_terrain_chunk_dimension;

void get_terrain_heightmap_size(int *w, int *h)
{
	*w = g_terrain_heightmap_width;
	*h = g_terrain_heightmap_height;
}

int set_terrain_chunk_dimension(int dimension)
{
	int d = dimension;
	if (even(dimension))
	{
		d = dimension+1;
	}
	g_terrain_chunk_dimension = d;
	return g_terrain_chunk_dimension;
}

int get_terrain_chunk_dimension(void)
{
	return g_terrain_chunk_dimension;
}

void B_update_terrain_chunk(TerrainChunk *chunk, uint64_t player_block_index)
{

	unsigned int texture = GL_TEXTURE0;	
	if (chunk->type == TERRAIN_CHUNK_WATER)
	{
		texture = GL_TEXTURE1;
	}
	int x_max = chunk->dimension/2;
	int x_offset = -x_max;
	int z_offset = -MAX_TERRAIN_BLOCKS*x_max;
	int x_counter = 0;
	int z_counter = 0;
	glUseProgram(chunk->compute_shader);
	glActiveTexture(texture);
	glBindTexture(GL_TEXTURE_2D, chunk->heightmap);
	if (chunk->type == TERRAIN_CHUNK_WATER)
	{
		B_set_uniform_int(chunk->compute_shader, "data", 1);
		glBindImageTexture(1, chunk->heightmap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
	else
	{
		B_set_uniform_int(chunk->compute_shader, "data", 0);
		glBindImageTexture(0, chunk->heightmap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
	B_set_uniform_float(chunk->compute_shader, "xz_scale", TERRAIN_XZ_SCALE);
	for (int i = 0; i < chunk->dimension*chunk->dimension; ++i)
	{
		int index = player_block_index + z_offset + x_offset;
		EnvironmentCondition environment_condition = get_environment_condition(index);
		B_set_uniform_int(chunk->compute_shader, "my_block_index", index);
		B_set_uniform_int(chunk->compute_shader, "x_counter", x_counter);
		B_set_uniform_int(chunk->compute_shader, "z_counter", z_counter);
		B_set_uniform_int(chunk->compute_shader, "temperature", environment_condition.temperature); 
		B_set_uniform_float(chunk->compute_shader, "precipitation", environment_condition.precipitation); 
		x_counter++;
		x_offset++;
		if (x_offset > x_max)
		{
			x_offset = -x_max;
			x_counter = 0;
			z_offset += MAX_TERRAIN_BLOCKS;
			z_counter++;
		}

		glDispatchCompute(chunk->width/8, chunk->height/8, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glBindTexture(GL_TEXTURE_2D, chunk->heightmap);
	glGenerateMipmap(GL_TEXTURE_2D);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, chunk->heightmap_buffer);
}

TerrainChunk create_terrain_chunk(unsigned int g_buffer, int type, unsigned long terrain_index)
{
	TerrainChunk chunk = {0};

	chunk.type = type;
	chunk.dimension = get_terrain_chunk_dimension();

	chunk.width = 64;
	chunk.height = 64;
	chunk.terrain_mesh = B_create_terrain_mesh(g_buffer);

	chunk.heightmap_width = chunk.width*chunk.dimension;
	chunk.heightmap_height = chunk.height*chunk.dimension;
	if (type == TERRAIN_CHUNK_LAND)
	{
		g_terrain_heightmap_width = chunk.heightmap_width;
		g_terrain_heightmap_height = chunk.heightmap_height;
		chunk.compute_shader = B_compile_compute_shader("render_progs/land_heightmap_gen_shader.comp");
	}
	else
	{
		chunk.compute_shader = B_compile_compute_shader("render_progs/water_heightmap_gen_shader.comp");
	}
	
	chunk.g_buffer = g_buffer;
	chunk.heightmap_size = chunk.heightmap_width * chunk.heightmap_height;
	chunk.heightmap_buffer = BG_MALLOC(TerrainHeight, chunk.heightmap_size);

	chunk.tessellation_level = 16.0;
	
	B_send_terrain_chunk_to_gpu(&chunk);

	B_update_terrain_chunk(&chunk, terrain_index);
	return chunk;
}

int get_terrain_heightmap_index_from_position(vec2 pos)
{
	int heightmap_width = 0;
	int heightmap_height = 0;
	get_terrain_heightmap_size(&heightmap_width, &heightmap_height); 

	size_t section_heightmap_height = round(heightmap_width/get_terrain_chunk_dimension());
	size_t section_heightmap_width = round(heightmap_height/get_terrain_chunk_dimension());
	
	float mesh_height = TERRAIN_XZ_SCALE*4;
	float mesh_width = TERRAIN_XZ_SCALE*4;

	float percent_x = glm_percent(0, mesh_width, pos[0]);
	float percent_z = glm_percent(0, mesh_height, pos[1]);

	int pixel_x = floor(section_heightmap_width * percent_x);
	int pixel_z = floor(section_heightmap_height * percent_z);

	pixel_x += section_heightmap_width;
	pixel_z += section_heightmap_height;

	return (pixel_z * heightmap_width) + pixel_x;
}

void B_send_terrain_chunk_to_gpu(TerrainChunk *chunk)
{
	unsigned int heightmap = 0;
	glGenTextures(1, &heightmap);
	glBindTexture(GL_TEXTURE_2D, heightmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, chunk->heightmap_width, chunk->heightmap_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	chunk->heightmap = heightmap;
}

TerrainMesh B_create_terrain_mesh(unsigned int g_buffer)
{
	int num_vertices = 4*4*4;
	T_Vertex vertices[num_vertices];

	memset(vertices, 0, sizeof(T_Vertex) * num_vertices);

	for (int i = 0; i < num_vertices; i += 4)
	{
		vertices[i].position[0] = -1.0;
		vertices[i].position[1] = 0.0;
		vertices[i].position[2] = -1.0;

		vertices[i+1].position[0] = -1.0;
		vertices[i+1].position[1] = 0.0;
		vertices[i+1].position[2] = 1.0;

		vertices[i+2].position[0] = 1.0;
		vertices[i+2].position[1] = 0.0;
		vertices[i+2].position[2] = 1.0;

		vertices[i+3].position[0] = 1.0;
		vertices[i+3].position[1] = 0.0;
		vertices[i+3].position[2] = -1.0;
	}

	TerrainMesh mesh = B_send_terrain_mesh_to_gpu(g_buffer, vertices, num_vertices, 4);
	return mesh;
}


void B_draw_water_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			uint64_t my_block_index, 
			uint64_t player_block_index, 
			float tessellation_level,
			B_Texture water_heightmap,
			float terrain_chunk_dimension,
			int heightmap_width,
			int heightmap_height,
			B_Texture land_heightmap,
			float camera_height)
{

	float time = (float)(SDL_GetTicks64()/150.0f);
	glUseProgram(shader);
	EnvironmentCondition cond = get_environment_condition(my_block_index);
	if (cond.precipitation < 0.2)
	{
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, land_heightmap);
	B_set_uniform_int(shader, "land_heightmap", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, water_heightmap);
	B_set_uniform_int(shader, "water_heightmap", 1);
	B_set_uniform_int(shader, "heightmap_width", heightmap_width);
	B_set_uniform_int(shader, "heightmap_height", heightmap_height);

	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_float(shader, "time", time);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_rows);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "my_block_index", my_block_index);
	B_set_uniform_int(shader, "player_block_index", player_block_index);
	B_set_uniform_float(shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_float(shader, "height_factor", 22.0f);
	B_set_uniform_float(shader, "sea_level", SEA_LEVEL);
	B_set_uniform_float(shader, "camera_height", camera_height);
	B_set_uniform_float(shader, "terrain_chunk_dimension", terrain_chunk_dimension);
	B_set_uniform_int(shader, "temperature", cond.temperature);

	if (USE_ALT_CAMERA)
	{
		mat4 alt_proj_view;
		get_alt_projection_view(alt_proj_view);
		vec3 frustum_corners[8];
		get_frustum_corners(alt_proj_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}

	}
	else
	{
		vec3 frustum_corners[8];
		get_frustum_corners(projection_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}
	}
	
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

void B_draw_terrain_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			uint64_t my_block_index, 
			uint64_t player_block_index, 
			float tessellation_level, 
			B_Texture heightmap,
			float terrain_chunk_dimension,
			int heightmap_width,
			int heightmap_height)
{
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);
	EnvironmentCondition cond = get_environment_condition(my_block_index);
	B_set_uniform_int(shader, "heightmap", 0);
	B_set_uniform_int(shader, "heightmap_width", heightmap_width);
	B_set_uniform_int(shader, "heightmap_height", heightmap_height);
	B_set_uniform_float(shader, "camera_height", get_camera_height());

	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_rows);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "my_block_index", my_block_index);
	B_set_uniform_int(shader, "player_block_index", player_block_index);
	B_set_uniform_float(shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_float(shader, "height_factor", TERRAIN_HEIGHT_FACTOR);
	B_set_uniform_int(shader, "temperature", cond.temperature);
	B_set_uniform_float(shader, "precipitation", cond.precipitation);
	B_set_uniform_float(shader, "sea_level", SEA_LEVEL);
	B_set_uniform_float(shader, "terrain_chunk_dimension", (float)terrain_chunk_dimension);
	B_set_uniform_int(shader, "draw_debug", 0);
	B_set_uniform_float(shader, "db_grass_patch_max_distance", 0.0f);
	for (int i = 0; i < 9; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "db_grass_patch_centers[%i]", i);
		B_set_uniform_vec3(shader, name, VEC3_ZERO);
	}

	if (USE_ALT_CAMERA)
	{
		vec3 frustum_corners[8];
		mat4 alt_proj_view;
		get_alt_projection_view(alt_proj_view);
		get_frustum_corners(alt_proj_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}

	}
	else
	{
		vec3 frustum_corners[8];
		get_frustum_corners(projection_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}
	}
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

void B_draw_terrain_mesh_debug(TerrainMesh mesh, 
				B_Shader shader, 
				mat4 projection_view,
				uint64_t my_block_index, 
				uint64_t player_block_index, 
				float tessellation_level, 
				B_Texture heightmap,
				float terrain_chunk_dimension,
				int heightmap_width,
				int heightmap_height,
				vec3 grass_patch_centers[9],
				float grass_patch_max_distance)
{
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);
	EnvironmentCondition cond = get_environment_condition(my_block_index);
	B_set_uniform_int(shader, "heightmap", 0);
	B_set_uniform_int(shader, "heightmap_width", heightmap_width);
	B_set_uniform_int(shader, "heightmap_height", heightmap_height);
	B_set_uniform_float(shader, "camera_height", get_camera_height());

	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_rows);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "my_block_index", my_block_index);
	B_set_uniform_int(shader, "player_block_index", player_block_index);
	B_set_uniform_float(shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_float(shader, "height_factor", TERRAIN_HEIGHT_FACTOR);
	B_set_uniform_int(shader, "temperature", cond.temperature);
	B_set_uniform_float(shader, "precipitation", cond.precipitation);
	B_set_uniform_float(shader, "sea_level", SEA_LEVEL);
	B_set_uniform_float(shader, "terrain_chunk_dimension", (float)terrain_chunk_dimension);
	B_set_uniform_int(shader, "draw_debug", 1);
	B_set_uniform_float(shader, "db_grass_patch_max_distance", grass_patch_max_distance);
	for (int i = 0; i < 9; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "db_grass_patch_centers[%i]", i);
		B_set_uniform_vec3(shader, name, grass_patch_centers[i]);
	}

	if (USE_ALT_CAMERA)
	{
		vec3 frustum_corners[8];
		mat4 alt_proj_view;
		get_alt_projection_view(alt_proj_view);
		get_frustum_corners(alt_proj_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}

	}
	else
	{
		vec3 frustum_corners[8];
		get_frustum_corners(projection_view, frustum_corners);
		for (int i = 0; i < 8; ++i)
		{
			char name[128] = {0};
			snprintf(name, 128, "frustum_corners[%i]", i);
			B_set_uniform_vec3(shader, name, frustum_corners[i]);
		}
	}
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

void get_block_corners(vec3 dest[4], int index)
{
	int dimension = get_terrain_chunk_dimension();
	int half_dimension = dimension/2;
	int x_index = index % dimension;
	int z_index = index / dimension;

	dest[0][0] = (x_index * (TERRAIN_XZ_SCALE*4)) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);
	dest[0][1] = 0;
	dest[0][2] = (z_index * (TERRAIN_XZ_SCALE*4)) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);

	dest[1][0] = (x_index+1) * (TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);
	dest[1][1] = 0;
	dest[1][2] = (z_index * (TERRAIN_XZ_SCALE*4)) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);

	dest[2][0] = (x_index * (TERRAIN_XZ_SCALE*4)) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);
	dest[2][1] = 0;
	dest[2][2] = (z_index+1) * (TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);

	dest[3][0] = (x_index+1) * (TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);
	dest[3][1] = 0;
	dest[3][2] = (z_index+1) * (TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4.0f*half_dimension);
}

void draw_land_terrain_chunk_debug(TerrainChunk *chunk, 
				   B_Shader shader, 
				   mat4 projection_view, 
				   uint64_t player_block_index, 
				   vec3 player_facing,
				   vec3 grass_patch_centers[9],
				   float grass_patch_max_distance)
{
	int x_max = chunk->dimension/2;
	int x_offset = -(x_max);
	int z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
	vec3 frustum_corners[8];
	if (USE_ALT_CAMERA)
	{
		mat4 alt_projv;
		get_alt_projection_view(alt_projv);
		get_frustum_corners(alt_projv, frustum_corners);
	}
	else
	{
		get_frustum_corners(projection_view, frustum_corners);
	}

	for (int i = 0; i < chunk->dimension*chunk->dimension; ++i)
	{
		uint64_t index = player_block_index + z_offset + x_offset;
		x_offset++;
		if (x_offset > x_max)
		{
			z_offset += MAX_TERRAIN_BLOCKS;
			x_offset = -x_max;
		}

		if (z_offset > MAX_TERRAIN_BLOCKS*x_max)
		{
			z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
		}

		/* If all four corners of the block are behind the player, don't draw. */
		vec3 block_corners[4] = {0};
		get_block_corners(block_corners, i);

		int corner_count = 0;
		for (int j = 0; j < 4; ++j)
		{
			block_corners[j][1] = get_raw_terrain_height_outside_bounds(block_corners[j], chunk);
			if (which_side(player_facing, frustum_corners[0], block_corners[j]))
			{
				corner_count++;
			}
		}
		if (corner_count >= 4)
		{
			continue;
		}
		
		if (chunk->type != TERRAIN_CHUNK_LAND)
		{
			fprintf(stderr, "B_draw_land_terrain_chunk error: invalid chunk type\n");
			exit(-1);
		}
		B_draw_terrain_mesh_debug(chunk->terrain_mesh,
					    shader,
					    projection_view,
					    index,
					    player_block_index,
					    chunk->tessellation_level,
					    chunk->heightmap,
					    chunk->dimension,
					    chunk->heightmap_width,
					    chunk->heightmap_height,
					    grass_patch_centers,
					    grass_patch_max_distance);

	}

}
void draw_land_terrain_chunk(TerrainChunk *chunk, B_Shader shader, mat4 projection_view, uint64_t player_block_index, vec3 player_facing)
{
	int x_max = chunk->dimension/2;
	int x_offset = -(x_max);
	int z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
	vec3 frustum_corners[8];
	if (USE_ALT_CAMERA)
	{
		mat4 alt_projv;
		get_alt_projection_view(alt_projv);
		get_frustum_corners(alt_projv, frustum_corners);
	}
	else
	{
		get_frustum_corners(projection_view, frustum_corners);
	}

	for (int i = 0; i < chunk->dimension*chunk->dimension; ++i)
	{
		uint64_t index = player_block_index + z_offset + x_offset;
		x_offset++;
		if (x_offset > x_max)
		{
			z_offset += MAX_TERRAIN_BLOCKS;
			x_offset = -x_max;
		}

		if (z_offset > MAX_TERRAIN_BLOCKS*x_max)
		{
			z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
		}

		/* If all four corners of the block are behind the player, don't draw. */
		vec3 block_corners[4] = {0};
		get_block_corners(block_corners, i);

		int corner_count = 0;
		for (int j = 0; j < 4; ++j)
		{
			block_corners[j][1] = get_raw_terrain_height_outside_bounds(block_corners[j], chunk);
			if (which_side(player_facing, frustum_corners[0], block_corners[j]))
			{
				corner_count++;
			}
		}
		if (corner_count >= 4)
		{
			continue;
		}
		
		if (chunk->type != TERRAIN_CHUNK_LAND)
		{
			fprintf(stderr, "B_draw_land_terrain_chunk error: invalid chunk type\n");
			exit(-1);
		}
		B_draw_terrain_mesh(chunk->terrain_mesh,
					    shader,
					    projection_view,
					    index,
					    player_block_index,
					    chunk->tessellation_level,
					    chunk->heightmap,
					    chunk->dimension,
					    chunk->heightmap_width,
					    chunk->heightmap_height);
	}

}
void draw_water_terrain_chunk(TerrainChunk *chunk, B_Texture land_heightmap, B_Shader shader, mat4 projection_view, uint64_t player_block_index, vec3 player_facing)
{
	int x_max = chunk->dimension/2;
	int x_offset = -(x_max);
	int z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
	vec3 frustum_corners[8];
	if (USE_ALT_CAMERA)
	{
		mat4 alt_projv;
		get_alt_projection_view(alt_projv);
		get_frustum_corners(alt_projv, frustum_corners);
	}
	else
	{
		get_frustum_corners(projection_view, frustum_corners);
	}

	for (int i = 0; i < chunk->dimension*chunk->dimension; ++i)
	{
		uint64_t index = player_block_index + z_offset + x_offset;
		x_offset++;
		if (x_offset > x_max)
		{
			z_offset += MAX_TERRAIN_BLOCKS;
			x_offset = -x_max;
		}

		if (z_offset > MAX_TERRAIN_BLOCKS*x_max)
		{
			z_offset = -(MAX_TERRAIN_BLOCKS*(x_max));
		}

		/* If all four corners of the block are behind the player, don't draw. */
		vec3 block_corners[4] = {0};
		get_block_corners(block_corners, i);

		int corner_count = 0;
		for (int j = 0; j < 4; ++j)
		{
			block_corners[j][1] = get_raw_terrain_height_outside_bounds(block_corners[j], chunk);
			if (which_side(player_facing, frustum_corners[0], block_corners[j]))
			{
				corner_count++;
			}
		}
		if (corner_count >= 4)
		{
			continue;
		}
		
		if (chunk->type != TERRAIN_CHUNK_WATER)
		{
			fprintf(stderr, "B_draw_water_terrain_chunk error: invalid chunk type\n");
			exit(-1);
		}
		B_draw_water_mesh(chunk->terrain_mesh,
					    shader,
					    projection_view,
					    index,
					    player_block_index,
					    chunk->tessellation_level,
					    chunk->heightmap,
					    chunk->dimension,
					    chunk->heightmap_width,
					    chunk->heightmap_height,
					    land_heightmap,
					    get_camera_height());

	}

}

TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int g_buffer, T_Vertex *vertices, int num_vertices, int num_rows)
{
	TerrainMesh mesh = {0};
	size_t stride = sizeof(GLfloat)*3;// + sizeof(GLfloat)*2;

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	mesh.num_vertices = num_vertices;
	mesh.num_rows = num_rows;
	mesh.g_buffer = g_buffer;
	mesh.use_ebo = 0;
	
	return mesh;
}

void B_free_terrain_mesh(TerrainMesh mesh)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteVertexArrays(1, &mesh.vao);
}

void free_terrain_chunk(TerrainChunk *chunk)
{
	for (int i = 0; i < chunk->dimension*chunk->dimension; ++i)
	{
		B_free_terrain_mesh(chunk->terrain_mesh);
	}
	B_Texture textures[2] = { chunk->heightmap, chunk->snow_normal_map };
	glDeleteTextures(2, textures);

	BG_FREE(chunk->heightmap_buffer);
}

unsigned int B_compile_compute_shader(const char *comp_path)
{
	unsigned int program_id = glCreateProgram();
	unsigned int compute_id = glCreateShader(GL_COMPUTE_SHADER);

	char compute_buffer[65336] = {0};
	B_load_file(comp_path, compute_buffer, 65336);
	const char *compute_source = compute_buffer;

	glShaderSource(compute_id, 1, &compute_source, NULL);
	glCompileShader(compute_id);
	B_check_shader(compute_id, comp_path, GL_COMPILE_STATUS);

	glAttachShader(program_id, compute_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);

	glDeleteShader(compute_id);
	return program_id;

}

void B_load_ai_terrain_mesh(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, TerrainMesh *mesh)
{
	TerrainVertexData vertex_data;
	memset(&vertex_data, 0, sizeof(TerrainVertexData));
	C_STRUCT aiMesh *a_mesh = scene->mMeshes[node->mMeshes[0]];

	vertex_data.num_vertices = a_mesh->mNumVertices;
	mesh->num_vertices = a_mesh->mNumVertices;
	vertex_data.vertices = BG_MALLOC(T_Vertex, a_mesh->mNumVertices);
	vertex_data.faces = NULL;
	for (unsigned int j = 0; j < a_mesh->mNumVertices; ++j)
	{
		vertex_data.vertices[j].position[0] = a_mesh->mVertices[j].x;
		vertex_data.vertices[j].position[1] = a_mesh->mVertices[j].y;
		vertex_data.vertices[j].position[2] = a_mesh->mVertices[j].z;

		mat4 transform;
		assimp_to_cglm_mat4(node->mTransformation, transform);
		glm_mat4_mulv3(transform, vertex_data.vertices[j].position, 1.0, vertex_data.vertices[j].position);

	}
	if (a_mesh->mNumFaces)
	{
		int num_elements = 0;
		for (unsigned int j = 0; j < a_mesh->mNumFaces; ++j)
		{
			C_STRUCT aiFace face = a_mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; ++k)
			{
				num_elements++;
			}
		} 
		vertex_data.num_faces = num_elements;
		vertex_data.faces = BG_MALLOC(unsigned int, num_elements);
		int i_counter = 0;
		for (unsigned int j = 0; j < a_mesh->mNumFaces; ++j)
		{
			C_STRUCT aiFace face = a_mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; ++k)
			{
				vertex_data.faces[i_counter++] = face.mIndices[k];
			}
		}
	}

	B_send_terrain_mesh_to_gpu_ebo(mesh, &vertex_data);
	BG_FREE(vertex_data.vertices);
	BG_FREE(vertex_data.faces);
}

void B_send_terrain_mesh_to_gpu_ebo(TerrainMesh *mesh, TerrainVertexData *vertex_data)
{
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	size_t stride = sizeof(GLfloat)*5;

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_data->num_vertices*stride, vertex_data->vertices, GL_DYNAMIC_DRAW);
	if (vertex_data->faces != NULL)
	{
		glGenBuffers(1, &mesh->ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_data->num_faces*sizeof(unsigned int), vertex_data->faces, GL_DYNAMIC_DRAW);
	}

	mesh->num_faces = vertex_data->num_faces;
	mesh->faces = BG_MALLOC(unsigned int, mesh->num_faces);
	memcpy(mesh->faces, vertex_data->faces, sizeof(unsigned int) * mesh->num_faces);
	mesh->use_ebo = 1;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

TerrainMesh load_terrain_mesh_from_file(B_Framebuffer g_buffer, const char *filename)
{
	TerrainMesh mesh;
	memset(&mesh, 0, sizeof(TerrainMesh));
	mesh.g_buffer = g_buffer;
	
	const C_STRUCT aiScene *scene = aiImportFile(filename, aiProcess_FlipUVs);
	if (scene == NULL)
	{
		fprintf(stderr, "load_terrain_mesh_from_file error: couldn't load file: %s\n", aiGetErrorString());
		exit(-1);
	}
	B_load_ai_terrain_mesh(scene, scene->mRootNode, &mesh);

	aiReleaseImport(scene);
	return mesh;	
}


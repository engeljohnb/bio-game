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

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <math.h>
#include <string.h>
#include "utils.h"
#include "terrain.h"

void B_update_terrain_block(TerrainBlock *block, int player_block_index)
{
	static int prev_player_block = 0;
	if (player_block_index == prev_player_block)
	{
		return;
	}

	int x_offset = -1;
	int z_offset =  -MAX_TERRAIN_BLOCKS;
	int x_counter = 0;
	int z_counter = 0;
	glUseProgram(block->compute_shader);
	for (int i = 0; i < 9; ++i)
	{
		int index = player_block_index + z_offset + x_offset;
		B_set_uniform_int(block->compute_shader, "my_block_index", index);
		B_set_uniform_float(block->compute_shader, "xz_scale", TERRAIN_XZ_SCALE);
		B_set_uniform_int(block->compute_shader, "x_counter", x_counter);
		B_set_uniform_int(block->compute_shader, "z_counter", z_counter);
		x_offset++;
		x_counter++;
		if (x_offset > 1)
		{
			z_offset += MAX_TERRAIN_BLOCKS;
			x_offset = -1;
			x_counter = 0;
			z_counter++;
		}

		if (z_offset > MAX_TERRAIN_BLOCKS)
		{
			z_offset = -MAX_TERRAIN_BLOCKS;
		}	

		glBindImageTexture(0, block->heightmap_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		glDispatchCompute(block->block_width/16, block->block_height/16, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glBindTexture(GL_TEXTURE_2D, block->heightmap_texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, block->heightmap_buffer);
	prev_player_block = player_block_index;
}

TerrainBlock create_terrain_block(unsigned int  g_buffer)
{
	TerrainBlock block;
	for (int i = 0; i < 9; ++i)
	{
		block.terrain_meshes[i] = B_create_terrain_mesh(g_buffer);
	}
	block.block_width = 64;
	block.block_height = 64;

	block.heightmap_width = block.block_width*3;;
	block.heightmap_height = block.block_height*3;
	block.compute_shader = B_compile_compute_shader("src/heightmap_gen_shader.comp");
	
	block.g_buffer = g_buffer;
	block.heightmap_size = block.heightmap_width * block.heightmap_height;
	block.heightmap_buffer = BG_MALLOC(GLfloat, block.heightmap_size);

	block.tessellation_level = 16.0;
	
	B_send_terrain_block_to_gpu(&block);

	int terrain_index = (MAX_TERRAIN_BLOCKS/4 * (MAX_TERRAIN_BLOCKS/2)) - (MAX_TERRAIN_BLOCKS/2);
	B_update_terrain_block(&block, terrain_index);
	return block;
}

void B_send_terrain_block_to_gpu(TerrainBlock *block)
{
	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, block->heightmap_width, block->heightmap_height, 0, GL_RED, GL_FLOAT, NULL);

	block->heightmap_texture = texture;
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

void B_draw_terrain_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			int my_block_index, 
			int player_block_index, 
			float tessellation_level, 
			B_Texture texture)
{

	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	B_set_uniform_int(shader, "heightmap", 0);
	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_columns);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "my_block_index", my_block_index);
	B_set_uniform_int(shader, "player_block_index", player_block_index);
	B_set_uniform_float(shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_float(shader, "height_scale", TERRAIN_HEIGHT_SCALE);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

void draw_terrain_block(TerrainBlock *block, B_Shader shader, mat4 projection_view, int player_block_index)
{
	glBindFramebuffer(GL_FRAMEBUFFER, block->g_buffer);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int x_offset = -1;
	int z_offset = -MAX_TERRAIN_BLOCKS;
	for (int i = 0; i < 9; ++i)
	{
		int index = player_block_index + z_offset + x_offset;
		x_offset++;
		if (x_offset > 1)
		{
			z_offset += MAX_TERRAIN_BLOCKS;
			x_offset = -1;
		}

		if (z_offset > MAX_TERRAIN_BLOCKS)
		{
			z_offset = -MAX_TERRAIN_BLOCKS;
		}

		B_draw_terrain_mesh(block->terrain_meshes[i],
				    shader,
				    projection_view,
				    index,
				    player_block_index,
				    block->tessellation_level,
				    block->heightmap_texture);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int g_buffer, T_Vertex *vertices, int num_vertices, int num_columns)
{
	TerrainMesh mesh = {0};
	size_t stride = sizeof(GLfloat)*3 + sizeof(GLfloat)*2;

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	mesh.num_vertices = num_vertices;
	//TODO: Technically you're interested in the number of rows, not the number of columns, so change this name.
	mesh.num_columns = num_columns;
	mesh.g_buffer = g_buffer;
	return mesh;
}

void B_free_terrain_mesh(TerrainMesh mesh)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteVertexArrays(1, &mesh.vao);
}

void free_terrain_block(TerrainBlock *block)
{
	for (int i = 0; i < 9; ++i)
	{
		B_free_terrain_mesh(block->terrain_meshes[i]);
	}

	BG_FREE(block->heightmap_buffer);
}

unsigned int B_compile_compute_shader(const char *comp_path)
{
	unsigned int program_id = glCreateProgram();
	unsigned int compute_id = glCreateShader(GL_COMPUTE_SHADER);

	char compute_buffer[65536] = {0};
	B_load_file(comp_path, compute_buffer, 65536);
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

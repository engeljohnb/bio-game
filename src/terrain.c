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
#include "time.h"
#include "noise.h"
#include "utils.h"
#include "terrain.h"

int g_terrain_heightmap_width;
int g_terrain_heightmap_height;

void get_terrain_heightmap_size(int *w, int *h)
{
	*w = g_terrain_heightmap_width;
	*h = g_terrain_heightmap_height;
}

void B_send_grass_blade_to_gpu(TerrainElementMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3;
	int num_vertices = 9*36;
	float depth = 0.20;
	float width = 0.08;

	GLfloat vertices_single_blade[] = 	
	{ 0, 		0, 		0, 
	  0,		0.25,		0,
	  0,		0.50,		depth/5,
	  0,		0.75,		depth/2,
	  width/2,	1,		depth,
	  width,	0.75,		depth/2,
	  width,	0.50,		depth/5,
	  width,	0.25,		0,
	  width,	0,		0 };

	unsigned int indices_single_blade[] = 
	{ 3, 4, 5,
	  2, 3, 5,
	  6, 2, 5,
	  1, 2, 6,
	  7, 1, 6,
	  0, 1, 7,
	  8, 0, 7 };

	GLfloat vertices[3 * 9 * 36] = { 0.0f };
	unsigned int indices[21 * 36] = { 0.0f };

	for (int i = 0; i < 36; ++i)
	{
		mat4 translation = GLM_MAT4_IDENTITY_INIT;
		mat4 rotation = GLM_MAT4_IDENTITY_INIT;
		float x = (float)(i % 6);
		float z = (float)floor(i / 6);
		float trans_x = noise1(x/4.0f) * x/1.5;
		float trans_z = noise1(z/4.0f) * z/1.5;

		float rotation_value = noise1((float)i/36.0f) * 10;

		glm_rotate(rotation, rotation_value, VEC3(0, 1, 0));
		glm_translate(translation, VEC3(trans_x, 0, trans_z));

		vec3 *grass_blade = (vec3 *)vertices_single_blade;
		vec3 *final_blades = (vec3 *)vertices;

		for (int j = 0; j < 9; ++j)
		{
			int index = i*9+j;
			mat4 transform;
			glm_mat4_mul(rotation, translation, transform);
			glm_mat4_mulv3(transform, grass_blade[j], 1.0f, final_blades[index]);
		}
		for (int j = 0; j < 21; ++j)
		{
			int index = i*21+j;
			indices[index] = indices_single_blade[j] + (i*9);
		}
	}

	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	mesh->num_elements = sizeof(indices)/sizeof(unsigned int);
	glGenBuffers(1, &mesh->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mesh->num_elements, indices, GL_STATIC_DRAW);

	mesh->num_vertices = num_vertices;
	mesh->shader = B_compile_grass_shader("src/grass_shader.vert", "src/grass_shader.geo", "src/grass_shader.frag");
}

TerrainElementMesh create_grass_blade(int g_buffer, B_Texture heightmap_texture)
{
	TerrainElementMesh mesh;
	memset(&mesh, 0, sizeof(TerrainElementMesh));
	mesh.g_buffer = g_buffer;
	mesh.heightmap_texture = heightmap_texture;
	B_send_grass_blade_to_gpu(&mesh);

	return mesh;
}

void B_update_terrain_chunk(TerrainChunk *block, int player_block_index)
{
	int x_offset = -1;
	int z_offset = -MAX_TERRAIN_BLOCKS;
	int x_counter = 0;
	int z_counter = 0;
	glUseProgram(block->compute_shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, block->heightmap_texture);
	B_set_uniform_int(block->compute_shader, "data", 0);
	B_set_uniform_float(block->compute_shader, "xz_scale", TERRAIN_XZ_SCALE);
	glBindImageTexture(0, block->heightmap_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	for (int i = 0; i < 9; ++i)
	{
		int index = player_block_index + z_offset + x_offset;
		B_set_uniform_int(block->compute_shader, "my_block_index", index);
		B_set_uniform_int(block->compute_shader, "x_counter", x_counter);
		B_set_uniform_int(block->compute_shader, "z_counter", z_counter);
		x_counter++;
		x_offset++;
		if (x_offset > 1)
		{
			x_offset = -1;
			x_counter = 0;
			z_offset += MAX_TERRAIN_BLOCKS;
			z_counter++;
		}

		glDispatchCompute(block->block_width/8, block->block_height/8, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glBindTexture(GL_TEXTURE_2D, block->heightmap_texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, block->heightmap_buffer);
}

TerrainChunk create_terrain_chunk(unsigned int  g_buffer)
{
	TerrainChunk block;
	for (int i = 0; i < 9; ++i)
	{
		block.terrain_meshes[i] = B_create_terrain_mesh(g_buffer);
	}
	block.block_width = 64;
	block.block_height = 64;

	block.heightmap_width = block.block_width*3;;
	block.heightmap_height = block.block_height*3;
	g_terrain_heightmap_width = block.heightmap_width;
	g_terrain_heightmap_height = block.heightmap_height;
	block.compute_shader = B_compile_compute_shader("src/heightmap_gen_shader.comp");
	
	block.g_buffer = g_buffer;
	block.heightmap_size = block.heightmap_width * block.heightmap_height;
	block.heightmap_buffer = BG_MALLOC(TerrainHeight, block.heightmap_size);

	block.tessellation_level = 16.0;
	
	B_send_terrain_chunk_to_gpu(&block);

	int terrain_index = (MAX_TERRAIN_BLOCKS/4 * (MAX_TERRAIN_BLOCKS/2)) - (MAX_TERRAIN_BLOCKS/2);
	B_update_terrain_chunk(&block, terrain_index);
	return block;
}

TerrainChunk create_server_terrain_chunk(void)
{
	TerrainChunk block;
	memset(&block, 0, sizeof(TerrainChunk));

	block.block_width = 64;
	block.block_height = 64;

	block.heightmap_width = block.block_width*3;;
	block.heightmap_height = block.block_height*3;
	block.compute_shader = B_compile_compute_shader("src/heightmap_gen_shader.comp");
	
	block.heightmap_size = block.heightmap_width * block.heightmap_height;
	block.heightmap_buffer = BG_MALLOC(TerrainHeight, block.heightmap_size);

	block.tessellation_level = 16.0;

	B_send_terrain_chunk_to_gpu(&block);

	int terrain_index = (MAX_TERRAIN_BLOCKS/4 * (MAX_TERRAIN_BLOCKS/2)) - (MAX_TERRAIN_BLOCKS/2);
	B_update_terrain_chunk(&block, terrain_index);
	return block;
}

int get_terrain_heightmap_index_from_position(vec2 pos)
{
	int heightmap_width = 0;
	int heightmap_height = 0;
	get_terrain_heightmap_size(&heightmap_width, &heightmap_height); 

	size_t section_heightmap_height = round(heightmap_width/3);
	size_t section_heightmap_width = round(heightmap_height/3);
	
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

void update_grass_patch_offset(vec2 offset, int index_diff)
{
	if (index_diff == 1)
	{
		offset[0] -= TERRAIN_XZ_SCALE*4;
	}
	if (index_diff == -1)
	{
		offset[0] += TERRAIN_XZ_SCALE*4;
	}
	if (index_diff == MAX_TERRAIN_BLOCKS)
	{
		offset[1] -= TERRAIN_XZ_SCALE*4;
	}
	if (index_diff == -MAX_TERRAIN_BLOCKS)
	{
		offset[1] += TERRAIN_XZ_SCALE*4;
	}
}

void B_send_terrain_chunk_to_gpu(TerrainChunk *block)
{
	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, block->heightmap_width, block->heightmap_height, 0, GL_RG, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

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

int get_grass_patch_size(unsigned int terrain_index)
{
	int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)x_index / MAX_TERRAIN_BLOCKS;
	float z = (float)z_index / MAX_TERRAIN_BLOCKS;

	return (round(noise2(x, z) * 180.0f));
}

void get_grass_patch_offset(unsigned int terrain_index, vec2 offset)
{
	int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	offset[0] = noise1((float)x_index/MAX_TERRAIN_BLOCKS) * TERRAIN_XZ_SCALE*4*3;
	offset[1] = noise1((float)z_index/MAX_TERRAIN_BLOCKS) * TERRAIN_XZ_SCALE*4*3; 
}

void get_grass_patch_offsets(unsigned int terrain_index, vec2 offsets[9])
{
	get_grass_patch_offset(terrain_index-MAX_TERRAIN_BLOCKS-1, offsets[0]);
	get_grass_patch_offset(terrain_index-MAX_TERRAIN_BLOCKS, offsets[1]);
	get_grass_patch_offset(terrain_index-MAX_TERRAIN_BLOCKS+1, offsets[2]);

	get_grass_patch_offset(terrain_index-1, offsets[3]);
	get_grass_patch_offset(terrain_index, offsets[4]);
	get_grass_patch_offset(terrain_index+1, offsets[5]);

	get_grass_patch_offset(terrain_index+MAX_TERRAIN_BLOCKS-1, offsets[6]);
	get_grass_patch_offset(terrain_index+MAX_TERRAIN_BLOCKS, offsets[7]);
	get_grass_patch_offset(terrain_index+MAX_TERRAIN_BLOCKS+1, offsets[8]);
}


void B_draw_grass_patch(TerrainElementMesh mesh, 
			mat4 projection_view, 
			vec3 player_position, 
			int x_offset, 
			int z_offset, 
			int patch_size, 
			float time, 
			vec2 base_offset)
{
	/* If base offset bleeds into another terrain block, don't draw.*/
	if ((base_offset[0] > TERRAIN_XZ_SCALE*4) ||
	    (base_offset[1] > TERRAIN_XZ_SCALE*4))
	{
		return;
	}

	vec2 offset;
	glm_vec2_copy(base_offset, offset);
	offset[0] += x_offset * TERRAIN_XZ_SCALE*4;
	offset[1] += z_offset * TERRAIN_XZ_SCALE*4;
	float view_distance = get_view_distance();
	/* If the grass is too far away to see, don't draw. */
	if ((player_position[0] - offset[0] > view_distance) ||
	    (player_position[2] - offset[1] > view_distance))
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh.heightmap_texture);
	mat4 scale;
	glm_mat4_identity(scale);
	glm_scale(scale, VEC3(8,8,8));
	B_set_uniform_int(mesh.shader, "heightmap", 0);
	B_set_uniform_float(mesh.shader, "patch_size", (float)patch_size);
	B_set_uniform_mat4(mesh.shader, "projection_view", projection_view);
	B_set_uniform_mat4(mesh.shader, "scale", scale);
	B_set_uniform_float(mesh.shader, "terrain_chunk_size", TERRAIN_XZ_SCALE*4);
	B_set_uniform_vec3(mesh.shader, "player_position", player_position);
	B_set_uniform_vec2(mesh.shader, "base_offset", offset);
	B_set_uniform_float(mesh.shader, "time", time);
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.num_elements, GL_UNSIGNED_INT, 0, patch_size*patch_size);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_grass_patches(TerrainElementMesh grass, 
			mat4 projection_view, 
			vec3 player_position, 
			unsigned int terrain_index, 
			vec2 offsets[9])
{
	static float time = 0.0f;
	int x_counter = -1;
	int z_counter = -1;
	for (int i = 0; i < 9; ++i)
	{
		unsigned int grass_terrain_index = terrain_index + x_counter + (z_counter * MAX_TERRAIN_BLOCKS);
		int patch_size = get_grass_patch_size(grass_terrain_index);
		B_draw_grass_patch(grass, projection_view, player_position, x_counter, z_counter, patch_size, time, offsets[i]);
		x_counter++;
		if (x_counter > 1)
		{
			x_counter = -1;
			z_counter++;
		}	
	}
	time += 1.0f;
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
	B_set_uniform_int(shader, "patches_per_column", mesh.num_rows);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "my_block_index", my_block_index);
	B_set_uniform_int(shader, "player_block_index", player_block_index);
	B_set_uniform_float(shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_float(shader, "height_scale", TERRAIN_HEIGHT_SCALE);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

void draw_terrain_chunk(TerrainChunk *block, B_Shader shader, mat4 projection_view, int player_block_index)
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

TerrainMesh B_send_terrain_mesh_to_gpu(unsigned int g_buffer, T_Vertex *vertices, int num_vertices, int num_rows)
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
	mesh.num_rows = num_rows;
	mesh.g_buffer = g_buffer;
	return mesh;
}

void B_free_terrain_element_mesh(TerrainElementMesh mesh)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteVertexArrays(1, &mesh.vao);
	//BG_FREE(mesh.offsets);
}

void B_free_terrain_mesh(TerrainMesh mesh)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteVertexArrays(1, &mesh.vao);
}

void free_terrain_chunk(TerrainChunk *block)
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

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
#include "utils.h"
#include "terrain.h"
#include "utils.h"

TerrainBlock create_terrain_block(B_Framebuffer g_buffer)
{
	TerrainBlock block;
	for (int i = 0; i < 9; ++i)
	{
		block.terrain_meshes[i] = B_create_terrain_mesh(g_buffer);
	}
	block.current_block_index = 4;
	return block;
}

TerrainMesh B_create_terrain_mesh(B_Framebuffer g_buffer)
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

TerrainMesh B_send_terrain_mesh_to_gpu(B_Framebuffer g_buffer, T_Vertex *vertices, int num_vertices, int num_columns)
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
	mesh.num_columns = num_columns;
	mesh.g_buffer = g_buffer;
	return mesh;
}

void B_draw_terrain_mesh(TerrainMesh mesh, B_Shader shader, Camera *camera, int block_index, float tessellation_level)
{
	glUseProgram(shader);

	mat4 projection_view;
	glm_mat4_mul(camera->projection_space, camera->view_space, projection_view);
	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_columns);
	B_set_uniform_float(shader, "tessellation_level", tessellation_level);
	B_set_uniform_int(shader, "current_block_index", (int)block_index);
	B_set_uniform_float(shader, "scale", SCALE);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}

int get_terrain_block_index(vec3 position)
{
	int x_index = (int)round(position[0])/(SCALE * 4);
	int z_index = (int)round(position[2])/(SCALE * 4);

	return (int)((z_index * MAX_X) + x_index);
}

void draw_terrain_block(TerrainBlock *block, B_Shader shader, Camera *camera)
{
	float tessellation_level = 16.0f;
	glBindFramebuffer(GL_FRAMEBUFFER, block->terrain_meshes[0].g_buffer);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int base_index = get_terrain_block_index(camera->position);
	int x_shift = -1;
	int y_shift = -MAX_X;

	for (int i = 0; i < 9; ++i)
	{
		int index = base_index + y_shift + x_shift;
		x_shift++;
		if (x_shift > 1)
		{
			x_shift = -1;
			y_shift += MAX_X;
		}
		if (y_shift > MAX_X)
		{
			y_shift = -MAX_X;
		}


		B_draw_terrain_mesh(block->terrain_meshes[i], shader, camera, index, tessellation_level);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
}

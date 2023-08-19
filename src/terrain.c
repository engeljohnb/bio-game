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
#include "asset_loading.h"
#include "environment.h"
#include "time.h"
#include "noise.h"
#include "utils.h"
#include "input.h"
#include "terrain.h"

int g_terrain_heightmap_width;
int g_terrain_heightmap_height;

void get_terrain_heightmap_size(int *w, int *h)
{
	*w = g_terrain_heightmap_width;
	*h = g_terrain_heightmap_height;
}

void B_update_terrain_chunk(TerrainChunk *block, uint64_t player_block_index)
{

	unsigned int texture = GL_TEXTURE0;	
	if (block->type == TERRAIN_CHUNK_WATER)
	{
		texture = GL_TEXTURE1;
	}
	int x_offset = -1;
	int z_offset = -MAX_TERRAIN_BLOCKS;
	int x_counter = 0;
	int z_counter = 0;
	glUseProgram(block->compute_shader);
	glActiveTexture(texture);
	glBindTexture(GL_TEXTURE_2D, block->heightmap_texture);
	B_set_uniform_int(block->compute_shader, "data", 0);
	B_set_uniform_float(block->compute_shader, "xz_scale", TERRAIN_XZ_SCALE);
	glBindImageTexture(0, block->heightmap_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	for (int i = 0; i < 9; ++i)
	{
		int index = player_block_index + z_offset + x_offset;
		EnvironmentCondition environment_condition = get_environment_condition(index);
		B_set_uniform_int(block->compute_shader, "my_block_index", index);
		B_set_uniform_int(block->compute_shader, "x_counter", x_counter);
		B_set_uniform_int(block->compute_shader, "z_counter", z_counter);
		B_set_uniform_int(block->compute_shader, "temperature", environment_condition.temperature); 
		B_set_uniform_float(block->compute_shader, "precipitation", environment_condition.precipitation); 
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
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, block->heightmap_buffer);
}

TerrainChunk create_terrain_chunk(unsigned int g_buffer, int type)
{
	TerrainChunk block;

	block.type = type;
	for (int i = 0; i < 9; ++i)
	{
		block.terrain_meshes[i] = B_create_terrain_mesh(g_buffer);
	}
	block.block_width = 64;
	block.block_height = 64;

	block.heightmap_width = block.block_width*3;
	block.heightmap_height = block.block_height*3;
	if (type == TERRAIN_CHUNK_LAND)
	{
		g_terrain_heightmap_width = block.heightmap_width;
		g_terrain_heightmap_height = block.heightmap_height;
		block.compute_shader = B_compile_compute_shader("render_progs/land_heightmap_gen_shader.comp");
	}
	else
	{
		block.compute_shader = B_compile_compute_shader("render_progs/water_heightmap_gen_shader.comp");
	}
	
	block.g_buffer = g_buffer;
	block.heightmap_size = block.heightmap_width * block.heightmap_height;
	block.heightmap_buffer = BG_MALLOC(TerrainHeight, block.heightmap_size);

	block.tessellation_level = 16.0;
	
	B_send_terrain_chunk_to_gpu(&block);

	B_update_terrain_chunk(&block, PLAYER_TERRAIN_INDEX_START);
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

void B_send_terrain_chunk_to_gpu(TerrainChunk *block)
{
	unsigned int heightmap_texture = 0;
	glGenTextures(1, &heightmap_texture);
	glBindTexture(GL_TEXTURE_2D, heightmap_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, block->heightmap_width, block->heightmap_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	block->heightmap_texture = heightmap_texture;
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
			B_Texture heightmap_texture,
			int heightmap_width,
			int heightmap_height,
			float camera_height)
{
	//TODO: Make it so all shaders that have a "time" parameter use the same time variable.
	static float time = 0.0f;
	glUseProgram(shader);
	EnvironmentCondition cond = get_environment_condition(my_block_index);
	if (cond.precipitation < 0.2)
	{
		return;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightmap_texture);
	B_set_uniform_int(shader, "heightmap", 1);
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
	B_set_uniform_int(shader, "temperature", cond.temperature);

	vec3 frustum_corners[8];
	get_frustum_corners(projection_view, frustum_corners);
	for (int i = 0; i < 8; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "frustum_corners[%i]", i);
		B_set_uniform_vec3(shader, name, frustum_corners[i]);
	}
	
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
	static int up = 1;
	if (up)
	{
		time += 0.01f;
	}
	else
	{
		time -= 0.01f;
	}
	if (time > 10000.0f)
	{
		if (up)
		{
			up = 0;
		}
		else
		{
			up = 1;
		}
	}
}

void B_draw_terrain_mesh(TerrainMesh mesh, 
			B_Shader shader, 
			mat4 projection_view,
			uint64_t my_block_index, 
			uint64_t player_block_index, 
			float tessellation_level, 
			B_Texture heightmap_texture,
			int heightmap_width,
			int heightmap_height)
{
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap_texture);
	EnvironmentCondition cond = get_environment_condition(my_block_index);
	B_set_uniform_int(shader, "heightmap", 0);
	B_set_uniform_int(shader, "heightmap_width", heightmap_width);
	B_set_uniform_int(shader, "heightmap_height", heightmap_height);

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

	vec3 frustum_corners[8];
	get_frustum_corners(projection_view, frustum_corners);
	for (int i = 0; i < 8; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "frustum_corners[%i]", i);
		B_set_uniform_vec3(shader, name, frustum_corners[i]);
	}
	
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}


void get_block_corners(vec3 dest[4], int index)
{
	int x_index = index % 3;
	int z_index = index / 3;

	dest[0][0] = (x_index * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);
	dest[0][1] = 0;
	dest[0][2] = (z_index * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);

	dest[1][0] = ((x_index+1) * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);
	dest[1][1] = 0;
	dest[1][2] = (z_index * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);

	dest[2][0] = (x_index * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);
	dest[2][1] = 0;
	dest[2][2] = ((z_index+1) * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);

	dest[3][0] = ((x_index+1) * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);
	dest[3][1] = 0;
	dest[3][2] = ((z_index+1) * TERRAIN_XZ_SCALE*4) - (TERRAIN_XZ_SCALE*4);
}

void draw_terrain_chunk(TerrainChunk *block, B_Shader shader, mat4 projection_view, uint64_t player_block_index, float camera_height)
{
	int x_offset = -1;
	int z_offset = -MAX_TERRAIN_BLOCKS;
	vec3 frustum_corners[8];
	get_frustum_corners(projection_view, frustum_corners);
	for (int i = 0; i < 9; ++i)
	{
		uint64_t index = player_block_index + z_offset + x_offset;
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

		vec3 player_facing;
		vec3 n_player_facing;
		{
			vec3 b_m_a;
			vec3 c_m_a;
			glm_vec3_sub(frustum_corners[1], frustum_corners[0], b_m_a);
			glm_vec3_sub(frustum_corners[2], frustum_corners[0], c_m_a);
			glm_vec3_cross(b_m_a, c_m_a, player_facing);
			glm_vec3_normalize(player_facing);
			glm_vec3_negate_to(player_facing, n_player_facing);
		}

		vec3 block_corners[4] = {0};
		get_block_corners(block_corners, i);
		int visible = 0;

		if (player_block_index == index)
		{
			visible = 1;
		}
		else
		{
			for (int j = 0; j < 4; ++j)
			{
				if ((which_side(player_facing, frustum_corners[1], block_corners[j])) &&
				    (which_side(n_player_facing, frustum_corners[7], block_corners[j])))
				{
					visible = 1;
					break;
				}
			}
		}

		if (visible)
		{
			if (block->type == TERRAIN_CHUNK_LAND)
			{
				B_draw_terrain_mesh(block->terrain_meshes[i],
						    shader,
						    projection_view,
						    index,
						    player_block_index,
						    block->tessellation_level,
						    block->heightmap_texture,
						    block->heightmap_width,
						    block->heightmap_height);
			}

			else if (block->type == TERRAIN_CHUNK_WATER)
			{
				B_draw_water_mesh(block->terrain_meshes[i],
						    shader,
						    projection_view,
						    index,
						    player_block_index,
						    block->tessellation_level,
						    block->heightmap_texture,
						    block->heightmap_width,
						    block->heightmap_height,
						    camera_height);
			}
			
		}
	}
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
	mesh.use_ebo = 0;
	
	return mesh;
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

		if (a_mesh->mTextureCoords[0] != NULL)
		{
			/* This is weird. Double check the assimp docs once you start using textures */
			vertex_data.vertices[j].tex_coords[0] = a_mesh->mTextureCoords[0][j].x;
			vertex_data.vertices[j].tex_coords[1] = a_mesh->mTextureCoords[0][j].y;
			//vertex_data.vertices[j].tex_coords[2] = 0.0f;
		}
		else
		{
			vertex_data.vertices[j].tex_coords[0] = 0.0f;
			vertex_data.vertices[j].tex_coords[1] = 0.0f;
			//vertex_data.vertices[j].tex_coords[2] = 0.0f;
		}

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


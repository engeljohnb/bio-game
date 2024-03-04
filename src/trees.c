#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "asset_loading.h"
#include "debug.h"
#include "environment.h"
#include "terrain_collisions.h"
#include "noise.h"
#include "terrain.h"
#include "trees.h"

void B_send_canopy_mesh_to_gpu(TerrainElementMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3;
	int num_vertices = 12*36;
	float depth = 0.20;
	float width = 0.08;

	GLfloat vertices_single_leaf[] = 	
	{
		-0.5,  0, -0.75, //L 0
		 0,   -1, 0, 	 //B 1
		 0.4,  0, -0.75, //R 2
		 0,    1, 0  	 //T 3
	};


	unsigned int indices_single_leaf[] = 
	{
		0, 1, 3,
		1, 2, 3	
	};

	GLfloat vertices[12*36] = { 0.0f };
	unsigned int indices[6 * 36] = { 0.0f };

	for (int i = 0; i < 36; ++i)
	{
		mat4 translation = GLM_MAT4_IDENTITY_INIT;
		mat4 rotation = GLM_MAT4_IDENTITY_INIT;
		float x = (float)(i % 6);
		float z = (float)floor(i / 6);
		float coefficient = 20.0f;

		float trans_x = noise1(x/4.0f) * x/1.5;
		float trans_y = noise2(x/4.0f, z/4.0f) * z/1.5;
		float trans_z = noise1(z/4.0f) * z/1.5;

		trans_x *= coefficient;
		trans_y *= coefficient;
		trans_z *= coefficient;

		glm_rotate(rotation, trans_y, VEC3(0, 1, 0));
		glm_rotate(rotation, trans_x, VEC3(1, 0, 0));
		glm_rotate(rotation, trans_z, VEC3(0, 0, 1));

		glm_translate(translation, VEC3(trans_x/5.0f, trans_y/5.0f, trans_z/5.0f));

		vec3 *leaf = (vec3 *)vertices_single_leaf;
		vec3 *final_patches = (vec3 *)vertices;

		for (int j = 0; j < 4; ++j)
		{
			int index = i*4+j;
			mat4 transform;
			glm_mat4_mul(rotation, translation, transform);
			glm_mat4_mulv3(transform, leaf[j], 1.0f, final_patches[index]);
		}
		for (int j = 0; j < 6; ++j)
		{
			int index = i*6+j;
			indices[index] = indices_single_leaf[j] + (i*4);
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
	mesh->shader = B_compile_simple_shader_with_geo("render_progs/canopy_shader.vert", "render_progs/canopy_shader.geo", "render_progs/canopy_shader.frag");
}

unsigned int get_canopy_size(EnvironmentCondition environment_condition, uint64_t terrain_index)
{
	int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)x_index / MAX_TERRAIN_BLOCKS;
	float z = (float)z_index / MAX_TERRAIN_BLOCKS;

	float size_factor = 1000.0f;

	if (environment_condition.temperature < 45)
	{
		size_factor *= glm_percent(0, 13, environment_condition.temperature-32);
	}
	if (environment_condition.precipitation < 0.2)
	{
		size_factor *= glm_percent(0, 0.2, environment_condition.precipitation);
	}
	return (unsigned int)(round(noise2(x, z) * size_factor));
}

void B_draw_canopy(Plant canopy, 
		   uint64_t terrain_index,
		   int mesh_id,
		   float scale_factor,
		   unsigned int size,
		   TerrainChunk *chunk,
		   vec2 base_offset, 
		   int x_offset,
		   int z_offset,
		   mat4 projection_view)
{
	/* If base offset bleeds into another terrain block, don't draw.*/
	if ((base_offset[0] > TERRAIN_XZ_SCALE*4) ||
	    (base_offset[1] > TERRAIN_XZ_SCALE*4))
	{
		return;
	}

	vec3 offset = GLM_VEC3_ZERO_INIT;
	offset[0] = base_offset[0] + (x_offset * (TERRAIN_XZ_SCALE*4));
	offset[2] = base_offset[1] + (z_offset * (TERRAIN_XZ_SCALE*4));
	offset[1] = get_terrain_height(offset, chunk) + 100.0f;

	float max_distance = 700.0f;
	if (USE_ALT_CAMERA)
	{
		mat4 alt_projv;
		get_alt_projection_view(alt_projv);
		if (!sphere_in_frustum(offset, max_distance, alt_projv))
		{
			return;
		}
	}
	else
	{
		if (!sphere_in_frustum(offset, max_distance, projection_view))
		{
			return;
		
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, canopy.meshes[mesh_id].g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, canopy.meshes[mesh_id].heightmap);

	B_set_uniform_float(canopy.meshes[mesh_id].shader, "max_distance", max_distance);
	B_set_uniform_uint(canopy.meshes[mesh_id].shader, "total", (unsigned int)size);
	B_set_uniform_float(canopy.meshes[mesh_id].shader, "scale_factor", scale_factor);
	B_set_uniform_uint(canopy.meshes[mesh_id].shader, "terrain_index", terrain_index);
	B_set_uniform_mat4(canopy.meshes[mesh_id].shader, "projection_view", projection_view);
	B_set_uniform_vec3(canopy.meshes[mesh_id].shader, "base_position", offset);
	B_set_uniform_int(canopy.meshes[mesh_id].shader, "num_subgroups", size/10);
	B_set_uniform_float(canopy.meshes[mesh_id].shader, "patch_size", (float)size);

	glBindVertexArray(canopy.meshes[mesh_id].vao);
	glDrawElementsInstanced(GL_TRIANGLES, canopy.meshes[mesh_id].num_elements, GL_UNSIGNED_INT, 0, size);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_canopy_meshes(int num_meshes, B_Framebuffer g_buffer, B_Texture heightmap, TerrainElementMesh dest[MAX_TERRAIN_ELEMENT_MESHES])
{
	for (int i = 0; i < num_meshes; ++i)
	{
		dest[i].g_buffer = g_buffer;
		dest[i].heightmap = heightmap;
		B_send_canopy_mesh_to_gpu(&dest[i]);
	}
}

Plant create_canopy(B_Framebuffer g_buffer, B_Texture heightmap)
{
	Plant canopy = {0};
	canopy.type = PLANT_TYPE_CANOPY;
	canopy.min_temperature = 0;
	canopy.max_temperature = 130;
	canopy.ideal_min_temperature = 55;
	canopy.ideal_max_temperature = 95;
	canopy.min_precipitation = 0.21f;
	canopy.max_precipitation = 1.0f;
	canopy.num_meshes = 4;
	canopy.scale_coefficients[0] = 2.0f;
	canopy.scale_coefficients[1] = 4.0f;
	canopy.scale_coefficients[2] = 6.0f;
	canopy.scale_coefficients[3] = 8.0f;
	create_canopy_meshes(canopy.num_meshes, g_buffer, heightmap, canopy.meshes);
	return canopy;
}

void B_send_generated_tree_trunk_mesh_to_gpu(TerrainElementMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3;
	int num_vertices = 4;

	GLfloat vertices[12] = { -1.0f, 0.0f,  1.0f,
       				 -1.0f, 0.0f, -1.0f,
				  1.0f, 0.0f, -1.0f,
				  1.0f, 0.0f,  1.0f };
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };

	glGenVertexArrays(1, &(mesh->vao));
	glBindVertexArray(mesh->vao);

	glGenBuffers(1, &(mesh->vbo));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	mesh->num_elements = sizeof(indices)/sizeof(unsigned int);
	glGenBuffers(1, &(mesh->ebo));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*mesh->num_elements, indices, GL_DYNAMIC_DRAW);

	mesh->num_vertices = num_vertices;
	mesh->shader = B_compile_simple_shader_with_geo("render_progs/tree_gen_shader.vert", "render_progs/tree_gen_shader.geo", "render_progs/tree_gen_shader.frag");
}

TerrainElementMesh create_generated_tree_trunk_mesh(B_Framebuffer g_buffer, B_Texture heightmap)
{
	TerrainElementMesh mesh = {0};
	mesh.g_buffer = g_buffer;
	mesh.heightmap = heightmap;
	B_send_generated_tree_trunk_mesh_to_gpu(&mesh);
	return mesh;
}

Plant B_create_generated_tree_trunk(B_Framebuffer g_buffer, B_Texture heightmap)
{
	Plant tree = {0};
	tree.type = PLANT_TYPE_GENERATED_TREE_TRUNK;
	tree.min_temperature = 0;
	tree.max_temperature = 130;
	tree.ideal_min_temperature = 55;
	tree.ideal_max_temperature = 95;
	tree.min_precipitation = 0.21f;
	tree.max_precipitation = 1.0f;
	tree.num_meshes = 1;
	tree.meshes[0] = create_generated_tree_trunk_mesh(g_buffer, heightmap);
	return tree;
}

void B_draw_generated_tree_trunk(Plant tree, 
		                 uint64_t terrain_index,
		                 int mesh_id,
		                 float scale_factor,
		                 TerrainChunk *chunk,
		                 vec2 base_offset, 
		                 int x_offset,
		                 int z_offset,
		                 mat4 projection_view)
{
	/* If base offset bleeds into another terrain block, don't draw.*/
	if ((base_offset[0] > TERRAIN_XZ_SCALE*4) ||
	    (base_offset[1] > TERRAIN_XZ_SCALE*4))
	{
		return;
	}

	vec3 offset = GLM_VEC3_ZERO_INIT;
	offset[0] = base_offset[0] + (x_offset * (TERRAIN_XZ_SCALE*4));
	offset[2] = base_offset[1] + (z_offset * (TERRAIN_XZ_SCALE*4));
	offset[1] = get_terrain_height(offset, chunk) + scale_factor;
	
	glBindFramebuffer(GL_FRAMEBUFFER, tree.meshes[mesh_id].g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tree.meshes[mesh_id].heightmap);

	unsigned int block_x = (int)(terrain_index % MAX_TERRAIN_BLOCKS) & 0xff;
	unsigned int block_z = (int)(terrain_index / MAX_TERRAIN_BLOCKS) & 0xff;
	unsigned int block = (block_x + block_z);

	B_set_uniform_uint(tree.meshes[mesh_id].shader, "block", (unsigned int)block/20);
	B_set_uniform_vec3(tree.meshes[mesh_id].shader, "base_offset", offset);
	B_set_uniform_mat4(tree.meshes[mesh_id].shader, "projection_view", projection_view);

	glBindVertexArray(tree.meshes[mesh_id].vao);

	if (tree.meshes[mesh_id].num_elements)
	{
		glDrawElementsInstanced(GL_TRIANGLES, tree.meshes[mesh_id].num_elements, GL_UNSIGNED_INT, 0, block/20);
	}
	else
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, tree.meshes[mesh_id].num_vertices, block/20);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void B_draw_tree_trunk(Plant tree, 
		   int mesh_id,
		   float scale_factor,
		   TerrainChunk *chunk,
		   vec2 base_offset, 
		   int x_offset,
		   int z_offset,
		   mat4 projection_view)
{
	/* If base offset bleeds into another terrain block, don't draw.*/
	if ((base_offset[0] > TERRAIN_XZ_SCALE*4) ||
	    (base_offset[1] > TERRAIN_XZ_SCALE*4))
	{
		return;
	}

	vec3 offset = GLM_VEC3_ZERO_INIT;
	offset[0] = base_offset[0] + (x_offset * (TERRAIN_XZ_SCALE*4));
	offset[2] = base_offset[1] + (z_offset * (TERRAIN_XZ_SCALE*4));
	offset[1] = get_terrain_height(offset, chunk) + scale_factor;
	
	glBindFramebuffer(GL_FRAMEBUFFER, tree.meshes[mesh_id].g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tree.meshes[mesh_id].heightmap);

	B_set_uniform_vec3(tree.meshes[mesh_id].shader, "base_offset", offset);
	B_set_uniform_float(tree.meshes[mesh_id].shader, "scale_factor", scale_factor);
	B_set_uniform_mat4(tree.meshes[mesh_id].shader, "projection_view", projection_view);

	glBindVertexArray(tree.meshes[mesh_id].vao);

	if (tree.meshes[mesh_id].num_elements)
	{
		glDrawElements(GL_TRIANGLES, tree.meshes[mesh_id].num_elements, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, tree.meshes[mesh_id].num_vertices);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Plant create_tree_trunk(B_Framebuffer g_buffer, B_Texture heightmap)
{
	Plant tree = {0};
	tree.type = PLANT_TYPE_TREE_TRUNK;
	tree.min_temperature = 0;
	tree.max_temperature = 130;
	tree.ideal_min_temperature = 55;
	tree.ideal_max_temperature = 95;
	tree.min_precipitation = 0.21f;
	tree.max_precipitation = 1.0f;
	tree.num_meshes = 1;
	tree.meshes[0] = load_plant_mesh_from_file("assets/trees/tree0.gltf", g_buffer, heightmap);
	return tree;
}

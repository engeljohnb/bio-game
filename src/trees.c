#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "terrain_collisions.h"
#include "noise.h"
#include "terrain.h"
#include "trees.h"

void B_send_canopy_mesh_to_gpu(TerrainElementMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3;
	int num_vertices = 9*36;
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

	GLfloat vertices[3 * 4 * 36] = { 0.0f };
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

void B_draw_canopy(Plant canopy, 
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
	offset[1] = get_terrain_height(offset, chunk) + 100.0;

	glBindFramebuffer(GL_FRAMEBUFFER, canopy.mesh.g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, canopy.mesh.heightmap);

	mat4 scale;
	glm_mat4_identity(scale);
	glm_scale(scale, VEC3(2,2,2));

	int patch_size = 1000;
	B_set_uniform_mat4(canopy.mesh.shader, "scale", scale);
	B_set_uniform_mat4(canopy.mesh.shader, "projection_view", projection_view);
	B_set_uniform_vec3(canopy.mesh.shader, "base_position", offset);
	B_set_uniform_int(canopy.mesh.shader, "num_subgroups", patch_size/10);
	B_set_uniform_float(canopy.mesh.shader, "patch_size", (float)patch_size);

	glBindVertexArray(canopy.mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, canopy.mesh.num_elements, GL_UNSIGNED_INT, 0, patch_size);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TerrainElementMesh create_canopy_mesh(int g_buffer, B_Texture heightmap)
{
	TerrainElementMesh mesh = {0};
	mesh.g_buffer = g_buffer;
	mesh.heightmap = heightmap;
	B_send_canopy_mesh_to_gpu(&mesh);

	return mesh;
}

Plant create_canopy(int g_buffer, B_Texture heightmap)
{
	Plant canopy = {0};
	canopy.type = PLANT_TYPE_CANOPY;
	canopy.min_temperature = 0;
	canopy.max_temperature = 130;
	canopy.ideal_min_temperature = 55;
	canopy.ideal_max_temperature = 95;
	canopy.min_precipitation = 0.21f;
	canopy.max_precipitation = 1.0f;
	canopy.mesh = create_canopy_mesh(g_buffer, heightmap);
	return canopy;
}

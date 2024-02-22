#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"
#include "grass.h"
#include "noise.h"
#include "utils.h"
#include "camera.h"
#include "debug.h"

// DEBUG
#include "input.h"

Plant create_grass_patch(B_Framebuffer g_buffer, B_Texture heightmap)
{
	Plant grass;
	memset(&grass, 0, sizeof(Plant));
	TerrainElementMesh mesh = create_grass_blade(g_buffer, heightmap);
	
	grass.mesh = mesh;
	grass.min_temperature = 32;
	grass.max_temperature = 140;
	grass.ideal_min_temperature = 45;
	grass.ideal_max_temperature = 90;
	grass.min_precipitation = 0.2;
	grass.max_precipitation = 1.0;
	return grass;
	
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

				//update_grass_patches(grass_patches, all_actors[i].actor_state.current_terrain_index);
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
	mesh->shader = B_compile_simple_shader_with_geo("render_progs/grass_shader.vert", "render_progs/grass_shader.geo", "render_progs/grass_shader.frag");
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

int get_grass_patch_size(EnvironmentCondition environment_condition, uint64_t terrain_index)
{
	int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)x_index / MAX_TERRAIN_BLOCKS;
	float z = (float)z_index / MAX_TERRAIN_BLOCKS;

	float size_factor = 230.0f;

	if (environment_condition.temperature < 45)
	{
		size_factor *= glm_percent(0, 13, environment_condition.temperature-32);
	}
	if (environment_condition.precipitation < 0.2)
	{
		size_factor *= glm_percent(0, 0.2, environment_condition.precipitation);
	}
	return (round(noise2(x, z) * size_factor));
}

void get_grass_patch_offset(uint64_t terrain_index, vec2 offset)
{
	int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	offset[0] = noise1((float)x_index/MAX_TERRAIN_BLOCKS) * TERRAIN_XZ_SCALE*4;
	offset[1] = noise1((float)z_index/MAX_TERRAIN_BLOCKS) * TERRAIN_XZ_SCALE*4; 
}

void get_grass_patch_offsets(uint64_t terrain_index, vec2 offsets[9])
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
			vec3 camera_position,
			TerrainChunk *chunk,
			mat4 projection_view,
			vec3 player_position, 
			vec3 player_facing,
			vec3 color,
			int x_offset, 
			int z_offset, 
			int patch_size, 
			vec2 base_offset)
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
	offset[1] = get_terrain_height(offset, chunk);
	float max_distance = TERRAIN_XZ_SCALE * 2.0f;
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

	float view_distance = get_view_distance();
	if ((x_offset == 0) && (z_offset == 0))
	{
		if (should_print_debug())
		{
			fprintf(stderr, "Corners:\n");
			for (int i = 0; i < 8; ++i)
			{
				vec3 d_vec;
				glm_vec3_scale(frustum_corners[i], 0.01f, d_vec);
				print_vec3(d_vec);
			}
			vec3 d_vec;
			fprintf(stderr, "Facing:\n");
			print_vec3(player_facing);
			fprintf(stderr, "Center:\n");
			glm_vec3_scale(offset, 0.01f, d_vec);
			print_vec3(d_vec);
			fprintf(stderr, "Radius: %f\n", max_distance/100.0f);
			fprintf(stderr, "Camera pos:\n");
			glm_vec3_scale(camera_position, 0.01f, d_vec);
			print_vec3(d_vec);
			fprintf(stderr, "==============================================\n\n");
		}
	}	

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

	float time = SDL_GetTicks64()/800.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh.heightmap);
	mat4 scale;
	glm_mat4_identity(scale);
	glm_scale(scale, VEC3(8,8,8));
	B_set_uniform_int(mesh.shader, "heightmap", 0);
	B_set_uniform_float(mesh.shader, "patch_size", (float)patch_size);
	B_set_uniform_mat4(mesh.shader, "projection_view", projection_view);
	B_set_uniform_float(mesh.shader, "terrain_chunk_size", TERRAIN_XZ_SCALE*4.0f);
	B_set_uniform_mat4(mesh.shader, "scale", scale);
	B_set_uniform_vec3(mesh.shader, "player_position", player_position);
	B_set_uniform_vec2(mesh.shader, "base_offset", VEC2(offset[0], offset[2]));
	B_set_uniform_float(mesh.shader, "time", time);
	B_set_uniform_vec3(mesh.shader, "player_facing", player_facing);
	B_set_uniform_vec3(mesh.shader, "color", color);
	B_set_uniform_float(mesh.shader, "view_distance", view_distance);
	B_set_uniform_float(mesh.shader, "max_distance", max_distance);
	B_set_uniform_float(mesh.shader, "sea_level", SEA_LEVEL);
	B_set_uniform_int(mesh.shader, "terrain_chunk_dimension", get_terrain_chunk_dimension());
	B_set_uniform_float(mesh.shader, "xz_scale", TERRAIN_XZ_SCALE);
	B_set_uniform_int(mesh.shader, "draw_debug", DRAW_DEBUG);

	for (int i = 0; i < 8; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "frustum_corners[%i]", i);
		B_set_uniform_vec3(mesh.shader, name, frustum_corners[i]);
	}
	
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.num_elements, GL_UNSIGNED_INT, 0, patch_size*patch_size);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_grass_patches(Plant grass_patch,
		vec3 camera_position,
			TerrainChunk *chunk,
			vec2 offsets[9],
			mat4 projection_view,
			vec3 player_position, 
			vec3 player_facing,
			uint64_t terrain_index)
{
	if (get_camera_height() < SEA_LEVEL)
	{
		return;
	}
	int x_counter = -1;
	int z_counter = -1;
	for (int i = 0; i < 9; ++i)
	{
		int draw = 1;
		uint64_t grass_terrain_index = terrain_index + x_counter + (z_counter * MAX_TERRAIN_BLOCKS);
		EnvironmentCondition environment_condition = get_environment_condition(grass_terrain_index);
		if ((environment_condition.temperature > grass_patch.max_temperature) ||
		    (environment_condition.temperature < grass_patch.min_temperature))
		{
			draw = 0;
		}
		if ((environment_condition.precipitation > grass_patch.max_precipitation) ||
		    (environment_condition.precipitation < grass_patch.min_precipitation))
		{
			draw = 0;
		}

		if (draw)
		{
			vec3 color;
			vec3 ideal_color;
			vec3 lo_temp_color;
			vec3 hi_temp_color;
			glm_vec3_copy(VEC3(0.19f, 0.23f, 0.058f), ideal_color);
			glm_vec3_copy(VEC3(0.3f, 0.05f, 0.02f), hi_temp_color);
			glm_vec3_copy(VEC3(0.0f, 0.026f, 0.036f), lo_temp_color);

			if (environment_condition.temperature > grass_patch.ideal_max_temperature)
			{
				float percent = glm_percent(grass_patch.ideal_max_temperature, grass_patch.max_temperature, environment_condition.temperature);
				glm_vec3_lerp(ideal_color, hi_temp_color, percent, color);
			}
			else if (environment_condition.temperature < grass_patch.ideal_min_temperature)
			{
				float percent = glm_percent(grass_patch.min_temperature, grass_patch.ideal_min_temperature, environment_condition.temperature);
				glm_vec3_lerp(lo_temp_color, ideal_color, percent, color);
			}
			else
			{
				glm_vec3_copy(ideal_color, color);
			}

			int patch_size = get_grass_patch_size(environment_condition, grass_terrain_index);
			B_draw_grass_patch(grass_patch.mesh, 
					   camera_position,
					   chunk,
					   projection_view, 
					   player_position, 
					   player_facing, 
					   color,
					   x_counter, 
					   z_counter, 
					   patch_size, 
					   offsets[i]);
		
		}
		x_counter++;
		if (x_counter > 1)
		{
			x_counter = -1;
			z_counter++;
		}
	}
}

TerrainElementMesh create_grass_blade(int g_buffer, B_Texture heightmap)
{
	TerrainElementMesh mesh;
	memset(&mesh, 0, sizeof(TerrainElementMesh));
	mesh.g_buffer = g_buffer;
	mesh.heightmap = heightmap;
	B_send_grass_blade_to_gpu(&mesh);

	return mesh;
}

void B_free_terrain_element_mesh(TerrainElementMesh mesh)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteVertexArrays(1, &mesh.vao);
}

void free_plant(Plant plant)
{
	B_free_terrain_element_mesh(plant.mesh);
}


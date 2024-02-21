#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "terrain_collisions.h"

vec3 g_interpolation_samples[4] = {0};

void set_current_interpolation_samples(vec3 interpolation_samples[4])
{
	for (int i = 0; i < 4; ++i)
	{
		g_interpolation_samples[i][0] = interpolation_samples[i][0];
		g_interpolation_samples[i][1] = interpolation_samples[i][1];
		g_interpolation_samples[i][2] = interpolation_samples[i][2];
	}
}

void get_current_interpolation_samples(vec3 interpolation_samples[4])
{
	for (int i = 0; i < 4; ++i)
	{
		interpolation_samples[i][0] = g_interpolation_samples[i][0];
		interpolation_samples[i][1] = g_interpolation_samples[i][1];
		interpolation_samples[i][2] = g_interpolation_samples[i][2];
	}
}

int get_pixel_index_from_position(vec3 pos, TerrainChunk *terrain_chunk)
{
	unsigned int total_heightmap_width = terrain_chunk->heightmap_width;
	unsigned int section_heightmap_width = terrain_chunk->width;
	unsigned int section_heightmap_height = terrain_chunk->height;

	float mesh_height = get_terrain_xz_scale()*4;
	float mesh_width = get_terrain_xz_scale()*4;

	float percent_x = glm_percent(0, mesh_width, pos[0]);
	float percent_z = glm_percent(0, mesh_height, pos[2]);

	int pixel_x = round(section_heightmap_width * percent_x);
	int pixel_z = round(section_heightmap_height * percent_z);

	int offset = terrain_chunk->dimension/2;
	pixel_x += section_heightmap_width*offset;
	pixel_z += section_heightmap_height*offset;

	return (pixel_z * total_heightmap_width) + pixel_x;
}

float get_height_from_pixel_index(unsigned int index, TerrainChunk *terrain_chunk)
{
	float final_height = terrain_chunk->heightmap_buffer[index].value * (terrain_chunk->heightmap_buffer[index].scale*TERRAIN_HEIGHT_FACTOR);
	if (terrain_chunk->heightmap_buffer[index].snow >= 0.36)
	{
		final_height += 2.5;
	}
	return final_height;
}

float get_raw_terrain_height_outside_bounds(vec3 pos, TerrainChunk *terrain_chunk)
{
	unsigned int total_heightmap_width = terrain_chunk->heightmap_width;
	unsigned int total_heightmap_height= terrain_chunk->heightmap_height;

	int half_dimension = (int)terrain_chunk->dimension/2;
	float mesh_height = (get_terrain_xz_scale()*4) * terrain_chunk->dimension;
	float mesh_width = (get_terrain_xz_scale()*4) * terrain_chunk->dimension;

	float percent_x = percent(-1200*half_dimension, mesh_width, pos[0]);
	float percent_z = percent(-1200*half_dimension, mesh_height, pos[2]);

	int pixel_x = round(total_heightmap_width * percent_x);
	int pixel_z = round(total_heightmap_height * percent_z);

	unsigned int index = (pixel_z * total_heightmap_width) + pixel_x;

	if (index > terrain_chunk->heightmap_size)
	{
		fprintf(stderr, "get_raw_terrain_height_outside_bounds error: Trying to access index %u from buffer of size %u.\n", index, terrain_chunk->heightmap_size);
		exit(-1);
	}

	float final_height = terrain_chunk->heightmap_buffer[index].value * (terrain_chunk->heightmap_buffer[index].scale*TERRAIN_HEIGHT_FACTOR);
	if (terrain_chunk->heightmap_buffer[index].snow >= 0.36)
	{
		final_height += 2.5;
	}
	return final_height;
}

float get_raw_terrain_height(vec3 pos, TerrainChunk *terrain_chunk)
{
	unsigned int total_heightmap_width = terrain_chunk->heightmap_width;
	unsigned int section_heightmap_width = terrain_chunk->width;
	unsigned int section_heightmap_height = terrain_chunk->height;

	float mesh_height = get_terrain_xz_scale()*4;
	float mesh_width = get_terrain_xz_scale()*4;

	float percent_x = percent(0, mesh_width, pos[0]);
	float percent_z = percent(0, mesh_height, pos[2]);

	int pixel_x = round(section_heightmap_width * percent_x);
	int pixel_z = round(section_heightmap_height * percent_z);

	int half_dimension = terrain_chunk->dimension/2;
	pixel_x += section_heightmap_width*half_dimension;
	pixel_z += section_heightmap_height*half_dimension;

	unsigned int index = (pixel_z * total_heightmap_width) + pixel_x;

	if (index > terrain_chunk->heightmap_size)
	{
		fprintf(stderr, "get_raw_terrain_height error: Trying to access index %u from buffer of size %u.\n", index, terrain_chunk->heightmap_size);
		exit(-1);
	}

	float final_height = terrain_chunk->heightmap_buffer[index].value * (terrain_chunk->heightmap_buffer[index].scale*TERRAIN_HEIGHT_FACTOR);
	if (terrain_chunk->heightmap_buffer[index].snow >= 0.36)
	{
		final_height += 2.5;
	}
	return final_height;
}


void snap_to_ground(vec3 pos, TerrainChunk *terrain_chunk)
{
	pos[1] = get_terrain_height(pos, terrain_chunk);
}

void get_four_nearest_heights(vec3 pos, TerrainChunk *terrain_chunk, vec4 dest)
{
	float grid_square_width = get_terrain_xz_scale() / terrain_chunk->tessellation_level;
	float grid_square_height = get_terrain_xz_scale() / terrain_chunk->tessellation_level;

	vec3 height0;
	vec3 height1;
	vec3 height2;
	vec3 height3;

	float grid_x_min = (pos[0] - fmodf(pos[0], grid_square_width));
	float grid_z_min = (pos[2] - fmodf(pos[2], grid_square_height));
	float grid_x_max = grid_x_min + grid_square_width;
	float grid_z_max = grid_z_min + grid_square_height;

	glm_vec3_copy(VEC3(grid_x_min, 0, grid_z_min), height0);
	glm_vec3_copy(VEC3(grid_x_min, 0, grid_z_max), height1);
	glm_vec3_copy(VEC3(grid_x_max, 0, grid_z_min), height2);
	glm_vec3_copy(VEC3(grid_x_max, 0, grid_z_max), height3);

	dest[0] = get_raw_terrain_height(height0, terrain_chunk);
	dest[1] = get_raw_terrain_height(height1, terrain_chunk);
	dest[2] = get_raw_terrain_height(height2, terrain_chunk);
	dest[3] = get_raw_terrain_height(height3, terrain_chunk);
}

float barrycentrically_interpolate(vec3 p1, vec3 p2, vec3 p3, vec3 pos)
{
		float det = (p2[2] - p3[2]) * (p1[0] - p3[0]) + (p3[0] - p2[0]) * (p1[2] - p3[2]);
		float l1 = ((p2[2] - p3[2]) * (pos[0] - p3[0]) + (p3[0] - p2[0]) * (pos[2] - p3[2])) / det;
		float l2 = ((p3[2] - p1[2]) * (pos[0] - p3[0]) + (p1[0] - p3[0]) * (pos[2] - p3[2])) / det;
		float l3 = 1.0 - l1 - l2;
		float result = l1 * p1[1] + l2 * p2[1] + l3 * p3[1];
		return result;
}


float get_terrain_height(vec3 pos, TerrainChunk *terrain_chunk)
{
	vec4 heights;
	get_four_nearest_heights(pos, terrain_chunk, heights);

	float grid_square_width = get_terrain_xz_scale() / terrain_chunk->tessellation_level;
	float grid_square_height = get_terrain_xz_scale() / terrain_chunk->tessellation_level;

	vec3 q00;
	vec3 q10;
	vec3 q11;
	vec3 q01;

	float grid_x_min = pos[0] - fmod(pos[0], grid_square_width);
	float grid_z_min = pos[2] - fmod(pos[2], grid_square_height);
	
	float grid_x_max = grid_x_min + grid_square_width;
	float grid_z_max = grid_z_min + grid_square_height;

	glm_vec3_copy(VEC3(0, heights[0], 0), q00);
	glm_vec3_copy(VEC3(0, heights[1], 1), q01);
	glm_vec3_copy(VEC3(1, heights[2], 0), q10);
	glm_vec3_copy(VEC3(1, heights[3], 1), q11);

	float player_grid_square_x = percent(grid_x_min, grid_x_max, pos[0]);
	float player_grid_square_z = percent(grid_z_min, grid_z_max, pos[2]);

        return bilinearly_interpolate_float(0, 1,
                                            0, 1,
                                            q00[1], q01[1],
                                            q10[1], q11[1],
                                            player_grid_square_x,
                                            player_grid_square_z);
}

void update_actor_gravity(ActorState *actor_state, float actor_height, TerrainChunk *terrain_chunk, float delta_t)
{
	float height = get_terrain_height(actor_state->position, terrain_chunk) + (actor_height/2.0f);
	if (actor_state->position[1] > height)
	{
		actor_state->position[1] -= 0.08 * delta_t;
	}
	if (actor_state->position[1] < height)
	{
		actor_state->position[1] = height;
	}	
}


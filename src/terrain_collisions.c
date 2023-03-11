#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "terrain_collisions.h"

void get_texel_from_position(vec3 pos, TerrainChunk *terrain_chunk, int *x, int *z)
{
	size_t section_heightmap_height = round(terrain_chunk->heightmap_width/3);
	size_t section_heightmap_width = round(terrain_chunk->heightmap_height/3);
	
	float mesh_height = TERRAIN_XZ_SCALE*4;
	float mesh_width = TERRAIN_XZ_SCALE*4;

	float percent_x = glm_percent(0, mesh_width, pos[0]);
	float percent_z = glm_percent(0, mesh_height, pos[2]);

	int pixel_x = floor(section_heightmap_width * percent_x);
	int pixel_z = floor(section_heightmap_height * percent_z);

	pixel_x += section_heightmap_width;
	pixel_z += section_heightmap_height;

	*x = pixel_x;
	*z = pixel_z;
}

float get_raw_terrain_height(vec3 pos, TerrainChunk *terrain_chunk)
{
	size_t total_heightmap_width = terrain_chunk->heightmap_width;
	size_t section_heightmap_height = round(terrain_chunk->heightmap_width/3);
	size_t section_heightmap_width = round(terrain_chunk->heightmap_height/3);

	float mesh_height = TERRAIN_XZ_SCALE*4;
	float mesh_width = TERRAIN_XZ_SCALE*4;

	float percent_x = glm_percent(0, mesh_width, pos[0]);
	float percent_z = glm_percent(0, mesh_height, pos[2]);

	int pixel_x = floor(section_heightmap_width * percent_x);
	int pixel_z = floor(section_heightmap_height * percent_z);

	pixel_x += section_heightmap_width;
	pixel_z += section_heightmap_height;

	unsigned int index = (pixel_z * total_heightmap_width) + pixel_x;
	return terrain_chunk->heightmap_buffer[index].value * (terrain_chunk->heightmap_buffer[index].scale*2500);
}

void snap_to_ground(vec3 pos, TerrainChunk *terrain_chunk)
{
	pos[1] = get_terrain_height(pos, terrain_chunk);
}

void find_nearest_grid_coord(vec3 pos, TerrainChunk *terrain_chunk, vec2 dest)
{
	vec2 player_coord;
	glm_vec2_copy(VEC2(pos[0], pos[2]), player_coord);

	int range_start = 0;
	int range_end = terrain_chunk->heightmap_size;
	int current_index = terrain_chunk->heightmap_size/2;
	for (size_t i = 0; i < terrain_chunk->heightmap_size; ++i)
	{
		float x0 = (float)(current_index % terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;
		float z0 = floor(current_index / terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;

		float x1 = (float)((current_index-1) % terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;
		float z1 = floor((current_index-1) / terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;

		float distance0 = glm_vec2_distance(VEC2(x0, z0), player_coord);
		float distance1 = glm_vec2_distance(VEC2(x1, z1), player_coord);

		if (distance0 >= distance1)
		{
			range_end -= (range_end - range_start)/2;
		}
		else
		{
			range_start += (range_end - range_start)/2;
		}
		if (range_start == range_end)
		{
			glm_vec3_copy(VEC2(x0, z0), dest);
			return;
		}
		current_index = range_end - ((range_end-range_start)/2);
	}

	float x = (float)(current_index % terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;
	float z = floor(current_index / terrain_chunk->heightmap_width) * TERRAIN_XZ_SCALE;
	glm_vec3_copy(VEC2(x, z), dest);
}

void find_nearest_grid_square(vec3 pos, TerrainChunk *terrain_chunk, vec2 dest)
{
	/* Size / num_vertices */
	float grid_square_size = (TERRAIN_XZ_SCALE * 4) / (4 * terrain_chunk->tessellation_level * 3);
	int x_index = (int)round(pos[0] / grid_square_size);
	int z_index = (int)round(pos[2] / grid_square_size);

	dest[0] = x_index * grid_square_size;
	dest[1] = z_index * grid_square_size;
}

void bilinearly_interpolate_vec3(vec3 q00,
				 vec3 q01,
				 vec3 q10,
				 vec3 q11,
				 vec3 pos,
				 vec3 dest)
{
	vec3 r0;
	vec3 r1;
	float scalar0 = (q10[0] - pos[0]) / (q10[0] - q00[0]);
	float scalar1 = (pos[0] - q00[0]) / (q10[0] - q00[0]);

	vec3 r0_part0;
	vec3 r0_part1;

	vec3 r1_part0;
	vec3 r1_part1;
	
	glm_vec3_scale(q00, scalar0, r0_part0);
	glm_vec3_scale(q10, scalar1, r0_part1);
	glm_vec3_scale(q01, scalar0, r1_part0);
	glm_vec3_scale(q11, scalar1, r1_part1);

	glm_vec3_add(r0_part0, r0_part1, r0);
	glm_vec3_add(r1_part0, r1_part1, r1);

	float r_scalar0 = (r1[2] - pos[2]) / (r1[2] - r0[2]);
	float r_scalar1 = (pos[2] - r0[2]) / (r1[2] - r0[2]);

	glm_vec3_scale(r0, r_scalar0, r0);
	glm_vec3_scale(r1, r_scalar1, r1);

	glm_vec3_add(r0, r1, dest);
}

float bilinearly_interpolate_float(float x0,
				   float x1,
				   float y0,
				   float y1,
				   float q00,
				   float q01,
				   float q10,
			           float q11,
			    	   float x,
				   float y)
{
	float scalar0x = (x1 - x) / (x1 - x0);
	float scalar1x = (x - x0) / (x1 - x0);

	float r0 = (q00 * scalar0x) + (q10 * scalar1x);
	float r1 = (q01 * scalar0x) + (q11 * scalar1x);

	float scalar0y = (y1 - y) / (y1 - y0);
	float scalar1y = (y - y0) / (y1 - y0);

	return (r0 * scalar0y) + (r1 * scalar1y);
}

void get_four_nearest_heights(vec3 pos, TerrainChunk *terrain_chunk, vec4 dest)
{
	float texel_width = TERRAIN_XZ_SCALE / terrain_chunk->tessellation_level;
	float texel_height = TERRAIN_XZ_SCALE / terrain_chunk->tessellation_level;

	vec3 height0;
	vec3 height1;
	vec3 height2;
	vec3 height3;

	float grid_x = (pos[0] - fmodf(pos[0], texel_width));
	float grid_z = (pos[2] - fmodf(pos[2], texel_height));

	glm_vec3_copy(VEC3(grid_x-texel_width, 0, grid_z-texel_height), height0);
	glm_vec3_copy(VEC3(grid_x, 0, grid_z-texel_height), height1);
	glm_vec3_copy(VEC3(grid_x, 0, grid_z), height2);
	glm_vec3_copy(VEC3(grid_x-texel_width, 0, grid_z), height3);

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

	float grid_square_width = TERRAIN_XZ_SCALE / terrain_chunk->tessellation_level;
	float grid_square_height = TERRAIN_XZ_SCALE / terrain_chunk->tessellation_level;

	vec3 q00;
	vec3 q10;
	vec3 q11;
	vec3 q01;

	float grid_x = pos[0] - fmodf(pos[0], grid_square_width);
	float grid_z = pos[2] - fmodf(pos[2], grid_square_height);

	glm_vec3_copy(VEC3(0, heights[0], 0), q00);
	glm_vec3_copy(VEC3(1, heights[1], 0), q10);
	glm_vec3_copy(VEC3(1, heights[2], 1), q11);
	glm_vec3_copy(VEC3(0, heights[3], 1), q01);

	float player_grid_square_x = glm_percent(grid_x, grid_x + grid_square_width, pos[0]);
	float player_grid_square_z = glm_percent(grid_z, grid_z + grid_square_height, pos[2]);

	float final_height = bilinearly_interpolate_float(q00[0], q10[0],
						q00[2], q01[2],
						q00[1], q01[1],
						q10[1], q11[1],
						player_grid_square_x,
						player_grid_square_z);
	//final_height *= TERRAIN_HEIGHT_SCALE;
	return final_height;
}

void update_actor_gravity(ActorState *actor_state, TerrainChunk *terrain_chunk, float delta_t)
{
	actor_state->position[0] -= 2.5;
	actor_state->position[2] -= 2.5;
	float height = get_terrain_height(actor_state->position, terrain_chunk) + 3.0f;
	if (actor_state->position[1] > height)
	{
		actor_state->position[1] -= 0.08 * delta_t;
	}
	if (actor_state->position[1] < height)
	{
		actor_state->position[1] = height;
	}
	actor_state->position[0] += 2.5;
	actor_state->position[2] += 2.5;
}


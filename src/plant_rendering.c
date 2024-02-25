#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "camera.h"
#include "environment.h"
#include "trees.h"
#include "grass.h"
#include "noise.h"
#include "plant_rendering.h"
#include "utils.h"

void draw_plants(Plant plant,
		 vec3 camera_position,
		 TerrainChunk *chunk,
		 vec2 *offsets,
		 int num_offsets,
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
	for (int i = 0; i < num_offsets; ++i)
	{
		int draw = 1;
		uint64_t plant_terrain_index = terrain_index + x_counter + (z_counter * MAX_TERRAIN_BLOCKS);
		EnvironmentCondition environment_condition = get_environment_condition(plant_terrain_index);
		if ((environment_condition.temperature > plant.max_temperature) ||
		    (environment_condition.temperature < plant.min_temperature))
		{
			draw = 0;
		}
		if ((environment_condition.precipitation > plant.max_precipitation) ||
		    (environment_condition.precipitation < plant.min_precipitation))
		{
			draw = 0;
		}

		if (draw)
		{

			//TODO: Make colors a part of the Plant struct
			vec3 color;
			vec3 ideal_color;
			vec3 lo_temp_color;
			vec3 hi_temp_color;
			glm_vec3_copy(VEC3(0.19f, 0.23f, 0.058f), ideal_color);
			glm_vec3_copy(VEC3(0.3f, 0.05f, 0.02f), hi_temp_color);
			glm_vec3_copy(VEC3(0.0f, 0.026f, 0.036f), lo_temp_color);

			if (environment_condition.temperature > plant.ideal_max_temperature)
			{
				float percent = glm_percent(plant.ideal_max_temperature, plant.max_temperature, environment_condition.temperature);
				glm_vec3_lerp(ideal_color, hi_temp_color, percent, color);
			}
			else if (environment_condition.temperature < plant.ideal_min_temperature)
			{
				float percent = glm_percent(plant.min_temperature, plant.ideal_min_temperature, environment_condition.temperature);
				glm_vec3_lerp(lo_temp_color, ideal_color, percent, color);
			}
			else
			{
				glm_vec3_copy(ideal_color, color);
			}

			int patch_size = get_grass_patch_size(environment_condition, plant_terrain_index);

			if (plant.type == PLANT_TYPE_GRASS)
			{
				unsigned int int_x = (plant_terrain_index % MAX_TERRAIN_BLOCKS);
				unsigned int int_z = (plant_terrain_index / MAX_TERRAIN_BLOCKS);

				float x = ((float)int_x / (float)MAX_TERRAIN_BLOCKS);
				float z = ((float)int_z / (float)MAX_TERRAIN_BLOCKS);

				float scale_factor = (1.0f + fbm2d((float)x*100, (float)z*100, 6, 0.60))/2.0f;
				scale_factor *= 20.0f;
				B_draw_grass_patch(plant.meshes[plant_terrain_index%plant.num_meshes], 
						   scale_factor,
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

			else if (plant.type == PLANT_TYPE_CANOPY)
			{
				unsigned int int_x = (plant_terrain_index % MAX_TERRAIN_BLOCKS);
				unsigned int int_z = (plant_terrain_index / MAX_TERRAIN_BLOCKS);
				float x = ((float)int_x / (float)MAX_TERRAIN_BLOCKS);
				float z = ((float)int_z / (float)MAX_TERRAIN_BLOCKS);
				float scale_factor = (1.0f + fbm2d((float)x*100, (float)z*100, 6, 0.60))/2.0f;
				scale_factor *= 10.0f;
				unsigned int canopy_size = get_canopy_size(environment_condition, plant_terrain_index);
				B_draw_canopy(plant, 
						plant_terrain_index,
						plant_terrain_index%plant.num_meshes, 
						scale_factor, 
						chunk, 
						offsets[i], 
						x_counter, 
						z_counter, 
						projection_view);
			}

		
		
		}
		x_counter++;
		if (x_counter > 1)
		{
			x_counter = -1;
			z_counter++;
		}
	}
}

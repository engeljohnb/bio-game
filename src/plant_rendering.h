#ifndef __PLANT_RENDERING_H__
#define __PLANT_RENDERING_H__

#include <cglm/cglm.h>
#include "plant.h"

void draw_plants(Plant grass_patch,
		 vec3 camera_position,
		 TerrainChunk *chunk,
		 vec2 *offsets,
		 int num_offsets,
		 mat4 projection_view,
		 vec3 player_position, 
		 vec3 player_facing,
		 uint64_t terrain_index);

#endif

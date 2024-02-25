#ifndef __TREES_H__
#define __TREES_H__
#include "plant.h"

Plant create_canopy(int g_buffer, B_Texture heightmap);
unsigned int get_canopy_size(EnvironmentCondition environment_condition, uint64_t terrain_index);
void B_draw_canopy(Plant canopy, 
		   uint64_t terrain_index, 
		   int mesh_id, 
		   float scale_factor, 
		   unsigned int size,
		   TerrainChunk *chunk, 
		   vec2 base_offset, 
		   int x_offset, 
		   int z_offset, 
		   mat4 projection_view);

#endif

#ifndef __TREES_H__
#define __TREES_H__
#include "plant.h"

Plant create_canopy(int g_buffer, B_Texture heightmap);
void B_draw_canopy(Plant canopy, TerrainChunk *chunk, vec2 base_offset, int x_offset, int z_offset, mat4 projection_view);

#endif

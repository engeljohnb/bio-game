#ifndef __GRASS_H__
#define __GRASS_H__

#include <cglm/cglm.h>
#include "common.h"
#include "terrain.h"

void update_grass_patches(Plant grass_patches[9], uint64_t player_terrain_index);
Plant create_grass_patch(vec2 xz_location, B_Framebuffer g_buffer, B_Texture heightmap);
void B_send_grass_blade_to_gpu(TerrainElementMesh *mesh);
void update_grass_patch_offset(vec2 offset, int index_diff);
int get_grass_patch_size(EnvironmentCondition environment_condition, uint64_t terrain_index);
void get_grass_patch_offset(uint64_t terrain_index, vec2 offset);
void get_grass_patch_offsets(uint64_t terrain_index, vec2 offsets[9]);
void B_draw_grass_patch(TerrainElementMesh mesh, 
			mat4 projection_view,
			vec3 player_position, 
			vec3 player_facing,
			vec3 color,
			int x_offset, 
			int z_offset, 
			int patch_size, 
			vec2 base_offset);

void draw_grass_patches(Plant grass_patches[9],
			mat4 projection_view,
			vec3 player_position, 
			vec3 player_facing,
			uint64_t terrain_index);

TerrainElementMesh create_grass_blade(int g_buffer, B_Texture heightmap);
void B_free_terrain_element_mesh(TerrainElementMesh mesh);
void free_plant(Plant plant);

#endif

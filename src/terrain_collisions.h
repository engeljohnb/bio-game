#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__
#include <cglm/cglm.h>
#include "actor_state.h"
#include "terrain.h"
float get_terrain_height(vec3 pos, TerrainChunk *block);

/* Gets the height, but doesn't interpoate between the values on the heightmap -- mostly needed as a
 * utility for get_terrain_height */
float get_raw_terrain_height(vec3 pos, TerrainChunk *terrain_chunk);
void update_actor_gravity(ActorState *actor_state, float actor_height, TerrainChunk *terrain_chunk, float delta_t);
void snap_to_ground(vec3 pos, TerrainChunk *terrain_chunk);
float get_raw_terrain_height_outside_bounds(vec3 pos, TerrainChunk *terrain_chunk);
void get_current_interpolation_samples(vec3 interpolation_samples[4]);

#endif

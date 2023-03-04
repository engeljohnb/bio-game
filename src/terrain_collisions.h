#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__
#include <cglm/cglm.h>
#include "actor_state.h"
#include "terrain.h"

float get_terrain_height(vec3 pos, TerrainBlock *block);

/* Gets the height, but doesn't interpoate between the values on the heightmap -- mostly needed as a
 * utility for get_terrain_height */
float get_raw_terrain_height(vec3 pos, TerrainBlock *terrain_block);
void update_actor_gravity(ActorState *actor_state, TerrainBlock *terrain_block, float delta_t);
void snap_to_ground(vec3 pos, TerrainBlock *terrain_block);

#endif

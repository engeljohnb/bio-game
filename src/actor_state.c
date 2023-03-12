/*
    Bio-Game is a game for designing your own microorganism. 
    Copyright (C) 2022 John Engel 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include "actor_rendering.h"
#include "utils.h"
#include "terrain.h"
#include "actor_state.h"
#include "terrain_collisions.h"
#include "actor.h"

#define GRAPHICAL_RENDER 1

void update_actor_state_direction(ActorState *actor_state, CommandState *command_state)
{
	glm_vec3_zero(command_state->move_direction);
	// NOTE: Why is actor_state->front pointing the wrong way?
	if (command_state->movement & M_BACKWARD)
	{
		glm_vec3_add(actor_state->front, command_state->move_direction, command_state->move_direction);
	}
	if (command_state->movement & M_FORWARD)
	{
		vec3 forward;
		glm_vec3_negate_to(actor_state->front, forward);
		glm_vec3_add(forward, command_state->move_direction, command_state->move_direction);
	}
	
	if (command_state->movement & M_LEFT)
	{
		vec3 left = {0.0, 0.0, 0.0};
		glm_vec3_negate_to(actor_state->right, left);
		glm_vec3_add(left, command_state->move_direction, command_state->move_direction);
	}
	if (command_state->movement & M_RIGHT)
	{
		glm_vec3_add(actor_state->right, command_state->move_direction, command_state->move_direction);
	}
	glm_vec3_normalize(command_state->move_direction);
	memcpy(&(actor_state->command_state), command_state, sizeof(CommandState));	
}

int actor_outside_terrain_boundaries(ActorState *actor_state)
{
	return ((actor_state->position[0] >= TERRAIN_XZ_SCALE*4-3) ||
		(actor_state->position[0] <= 3) ||
		(actor_state->position[2] >= TERRAIN_XZ_SCALE*4-3) ||
		(actor_state->position[2] <= 3));

}
void update_actor_state_position(ActorState *actor_state, CommandState command_state, float delta_t)
{
	if (command_state.movement)
	{
		actor_state->speed += 0.005*delta_t;
	}
	else	
	{
		glm_vec3_copy(VEC3_ZERO, command_state.move_direction);
		actor_state->speed = 0;
	}
	if (actor_state->speed > actor_state->max_speed)
	{
		actor_state->speed = actor_state->max_speed;
	}
	
	if (actor_state->speed < 0)
	{
		actor_state->speed = 0;
	}

	mat4 rotation_dest;
	get_rotation_matrix(command_state.look_x, command_state.look_y, rotation_dest);
	glm_mat4_mulv3(rotation_dest, VEC3_Z_DOWN, 0, actor_state->front);
	glm_mat4_mulv3(rotation_dest, VEC3_X_DOWN, 0, actor_state->right);

	vec3 velocity;
	vec3 move_direction_xz;
	glm_vec3_copy(VEC3(command_state.move_direction[0], 0, command_state.move_direction[2]), move_direction_xz);
	glm_vec3_scale(move_direction_xz, actor_state->speed, velocity);
	glm_vec3_add(actor_state->position, velocity, actor_state->position);

	if (actor_state->position[0] > TERRAIN_XZ_SCALE*4)
	{
		actor_state->position[0] = 0;
		actor_state->current_terrain_index += 1;
	}
	if (actor_state->position[0] < 0)
	{
		actor_state->position[0] = TERRAIN_XZ_SCALE*4;
		actor_state->current_terrain_index -= 1;
	}

	if (actor_state->position[2] > TERRAIN_XZ_SCALE*4)
	{
		actor_state->position[2] = 0;
		actor_state->current_terrain_index += MAX_TERRAIN_BLOCKS;
	}
	if (actor_state->position[2] < 0)
	{
		actor_state->position[2] = TERRAIN_XZ_SCALE*4;
		actor_state->current_terrain_index -= MAX_TERRAIN_BLOCKS;
	}
}


ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing)
{
	ActorState state;
	memset(&state, 0, sizeof(ActorState));
	memset(&state.command_state, 0, sizeof(CommandState));
	glm_vec3_copy(position, state.position);
	glm_vec3_copy(facing, state.front);
	state.speed = 0;
	// Actor begins roughly in the middle of the map.
	state.current_terrain_index = (MAX_TERRAIN_BLOCKS/4 * (MAX_TERRAIN_BLOCKS/2)) - (MAX_TERRAIN_BLOCKS/2);
	//state.prev_terrain_index = state.current_terrain_index;
	//state.current_terrain_index = 0;
	state.max_speed = 1.7;
	state.active = 1;
	state.id = id;
	return state;
}


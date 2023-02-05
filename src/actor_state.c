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
#include "actor_state.h"
#include "actor.h"

#define GRAPHICAL_RENDER 1

void update_actor_state_direction(ActorState *actor_state, CommandState *command_state)
{
	glm_vec3_zero(command_state->move_direction);
	if (command_state->movement & M_BACKWARD)
	{
		vec3 backward;
		glm_vec3_negate_to(actor_state->front, backward);
		glm_vec3_add(backward, command_state->move_direction, command_state->move_direction);
	}
	if (command_state->movement & M_FORWARD)
	{
		glm_vec3_add(actor_state->front, command_state->move_direction, command_state->move_direction);
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

	vec3 velocity;
	turn(actor_state->front, command_state.look_x, 0, VEC3_Z_UP, NULL);
	turn(actor_state->right, command_state.look_x, 0, VEC3_X_DOWN, NULL);
	glm_vec3_scale(command_state.move_direction, actor_state->speed, velocity);
	glm_vec3_add(actor_state->position, velocity, actor_state->position);
}

ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing)
{
	ActorState state;
	memset(&state, 0, sizeof(ActorState));
	memset(&state.command_state, 0, sizeof(CommandState));
	glm_vec3_copy(position, state.position);
	glm_vec3_copy(position, state.position);
	glm_vec3_copy(facing, state.front);
	state.speed = 0;
	state.max_speed = 3.0;
	state.active = 1;
	state.id = id;
	return state;
}


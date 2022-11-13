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
#include "graphics.h"
#include "utils.h"
#include "actor_state.h"
#include "actor.h"

#define GRAPHICAL_RENDER 1

void update_actor_state(ActorState *actor_state, CommandState command_state, float delta_t)
{
	memcpy(&(actor_state->command_state), &command_state, sizeof(CommandState));	
	actor_state->id = command_state.id;
	glm_vec3_negate(command_state.move_direction);
	glm_vec3_copy(actor_state->front, actor_state->prev_front);
	glm_vec3_copy(command_state.move_direction, actor_state->front);
	if (command_state.movement & M_BACKWARD)
	{
		/*vec3 backward = {0.0, 0.0, 0.0};
		glm_vec3_negate_to(actor_state->front, backward);
		glm_normalize(backward);
		glm_vec3_add(backward, command_state.move_direction, command_state.move_direction);
		glm_vec3_normalize(command_state.move_direction);*/
		glm_vec3_negate(command_state.move_direction);
	}
	if (command_state.movement & M_FORWARD)
	{
		vec3 forward;
		glm_vec3_copy(actor_state->front, forward);
		glm_vec3_normalize(forward);
		glm_vec3_add(forward, command_state.move_direction, command_state.move_direction);
		glm_vec3_normalize(command_state.move_direction);
	}
	
	if (command_state.movement & M_LEFT)
	{
		vec3 left = {0.0, 0.0, 0.0};
		glm_vec3_cross(actor_state->front, actor_state->up, left);
		glm_vec3_negate(left);
		glm_vec3_normalize(left);
		glm_vec3_add(left, command_state.move_direction, command_state.move_direction);
		glm_vec3_normalize(command_state.move_direction);
	}
	if (command_state.movement & M_RIGHT)
	{
		vec3 right = {0.0, 0.0, 0.0};
		glm_vec3_cross(actor_state->front, actor_state->up, right); glm_normalize(right);
		glm_vec3_add(right, command_state.move_direction, command_state.move_direction);
		glm_vec3_normalize(command_state.move_direction);
	}

	if (command_state.movement)
	{
		actor_state->speed += 0.0001*delta_t;
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
	if (!actor_state->num_updates)
	{
		glm_vec3_copy(actor_state->position, actor_state->prev_position);
	}
	glm_vec3_scale(command_state.move_direction, actor_state->speed, velocity);
	glm_vec3_add(actor_state->position, velocity, actor_state->position);
	actor_state->num_updates++;
}

ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing)
{
	ActorState state;
	memset(&state, 0, sizeof(ActorState));
	memset(&state.command_state, 0, sizeof(CommandState));
	glm_vec3_copy(position, state.position);
	glm_vec3_copy(position, state.prev_position);
	glm_vec3_copy(facing, state.front);
	glm_vec3_copy(VEC3_Y_UP, state.up);
	state.speed = 0;
	state.active = 1;
	state.max_speed = 0.3;
	state.id = id;
	return state;
}


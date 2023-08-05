/*
    Bio-Game is a game for designing your own organism. 
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



#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__
#include <cglm/cglm.h>
#include "input.h"
#define MAX_PLAYERS 4


/* An ActorState is all data needed about an actor to advance the simulation, including user input.
 * It probably could have been combined with the Actor object itself, but it's a leftover from when
 * the game was intended to be multiplayer. The ActorState was to be the minimum information
 * that needed to be sent over the network. */
typedef struct
{
	vec3			position;
	vec3			front;
	vec3			right;
	CommandState		command_state;
	int			active;
	uint64_t		current_terrain_index;			
	uint64_t		prev_terrain_index;
	unsigned int		id;
	float			speed;
	float			max_speed;
} ActorState;

ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing);

/* Applies the changes to an ActorState commanded by the command state */
void update_actor_state_position(ActorState *actor_state, CommandState command_state, float delta_t);
void update_actor_state_direction(ActorState *actor_state, CommandState *command_state);
int actor_outside_terrain_boundaries(ActorState *actor_state);

#endif

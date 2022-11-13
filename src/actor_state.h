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



#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__
#include <cglm/cglm.h>
#include "input.h"
#define MAX_PLAYERS 4

/* An ActorState is the minimum information about an actor that needs to be sent over the network.
 * Graphics and rendering are all handled client-side, while the server uses this struct to
 * advance the simulation */
typedef struct
{
	vec3			position;
	vec3			prev_position;
	vec3			front;
	vec3			up;
	CommandState		command_state;
	int			active;
	unsigned int		id;
	int			num_updates;
	float			speed;
	float			max_speed;
} ActorState;


/* A NewPlayerPackage is sent to a new player as soon as they join the game. It contains the current
 * state of each other player. */
typedef struct
{
	ActorState	actor_states[MAX_PLAYERS];
	unsigned int	num_actors;
	unsigned int	my_id;
} NewPlayerPackage;

ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing);

/* Destructively applies changes based on the given CommandState */
void update_actor_state(ActorState *actor_state, CommandState command_state, float delta_t);

#endif

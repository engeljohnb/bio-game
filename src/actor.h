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

#ifndef __ACTORS_H__
#define __ACTORS_H__
#include <SDL2/SDL.h>
#include "graphics.h"
#include "actor_state.h"
#include "input.h"

#define CURRENT_PLAYER 0

/* An actor is a player or an NPC. The Actor struct has all information about the actor, including graphics,
 * while the ActorState struct (see actor_state.h) has only the information needed for network communication. */
typedef struct
{
	unsigned int	id;
	CommandConfig	command_config;
	ActorState	actor_state;
	B_Model		model;
} Actor;

Actor create_player(unsigned int id);
void update_actor(Actor *actor, ActorState actor_state);
void free_actor(Actor actor);
Actor create_default_npc(unsigned int id);
void render_game(Actor *all_actors, unsigned int num_actors, Renderer renderer);
#endif

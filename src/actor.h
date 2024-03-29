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

#ifndef __ACTORS_H__
#define __ACTORS_H__
#include <SDL2/SDL.h>
#include "actor_rendering.h"
#include "actor_state.h"
#include "input.h"

#define CURRENT_PLAYER 0

enum
{
	ACTOR_ACTION_WALK
};

/* An actor is a player or an NPC. The Actor struct has all information about the actor, including graphics,
 * while the ActorState struct (see actor_state.h) has only the information needed for network communication. */
typedef struct Actor
{
	unsigned int	id;
	CommandConfig	command_config;
	ActorState	actor_state;
	ActorModel	*model;
	Animation	**animations;
	int		num_animations;
} Actor;

Actor create_player(unsigned int id);
void update_actor_model(ActorModel *model, ActorState actor_state);
void update_actor(Actor *actor, ActorState actor_state);
void B_draw_actors(Actor *all_actors, B_Shader shader, unsigned int num_actors, Renderer renderer);
void free_actor(Actor actor);
Actor create_default_npc(unsigned int id);
void set_actor_action(Actor *actor, int action);
#endif

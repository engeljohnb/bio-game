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
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include "graphics.h"
#include "gamestate.h"
#include "actor.h"
#include "input.h"
#include "utils.h"

Actor create_player(unsigned int id)
{
	Actor player;
	memset(&player, 0, sizeof(Actor));
	memset(&player.model, 0, sizeof(B_Model));
	player.actor_state = create_actor_state(id, VEC3_ZERO, VEC3_Z_UP);
	player.model = load_model_from_file("assets/monkey.bgm");
	player.id = id;
	player.model.valid = 1;
	player.command_config = default_command_config();
	return player;
}

Actor create_default_npc(unsigned int id)
{
	Actor actor;
	memset(&actor, 0, sizeof(Actor));
	memset(&actor.model, 0, sizeof(B_Model));
	actor.actor_state = create_actor_state(id, VEC3(0, 0, -5), VEC3_Z_UP);
	actor.id = id;
	actor.command_config = default_command_config();
	actor.model = load_model_from_file("assets/monkey.bgm");
	glm_translate(actor.model.world_space, VEC3(0.0, 0.0, 0.0));
	glm_translate(actor.model.world_space, VEC3(0.0, 0.0, 0.0));
	return actor;
}

void update_actor(Actor *actor, ActorState actor_state)
{
	memcpy(&actor->actor_state, &actor_state, sizeof(ActorState));
	vec3 position_dif;
	glm_vec3_sub(actor_state.position, actor_state.prev_position, position_dif);
	glm_translate(actor->model.world_space, position_dif);
	//glm_mat4_mulv3(actor->model.world_space, actor_state.position, 1, actor_state.position);
}

void render_game(Actor *all_actors, unsigned int num_actors, Renderer renderer)
{
	B_clear_window(renderer.window);
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		if (!(all_actors[i].model.valid))
		{
			continue;
		}
		B_blit_model(all_actors[i].model, renderer.camera, renderer.shader, renderer.point_light);
	}
	B_flip_window(renderer.window);
}


void free_actor(Actor actor)
{
	B_free_model(actor.model);
}

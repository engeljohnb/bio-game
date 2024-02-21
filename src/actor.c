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

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include "actor_rendering.h"
#include "asset_loading.h"
#include "actor_state.h"
#include "actor.h"
#include "input.h"
#include "terrain_collisions.h"
#include "utils.h"

Actor create_player(unsigned int id)
{
	Actor player;
	memset(&player, 0, sizeof(Actor));
	player.model = NULL;
	// Each tile has width and height of get_terrain_xz_scale()*4, so player starts in the center of the current tile.
	vec3 position;
	glm_vec3_copy(VEC3(get_terrain_xz_scale()*2, 0, get_terrain_xz_scale()*2), position);
	player.actor_state = create_actor_state(id, position, VEC3_Z_UP);
	player.model = B_load_model_from_file("assets/monkey/monkey.gltf");
	player.animations = B_load_animations_from_file("assets/monkey/monkey.gltf", &player.num_animations);
	if (player.animations == NULL)
	{
		player.model->current_animation = NULL;
	}
	else
	{
		player.model->current_animation = player.animations[0];
	}
	player.id = id;
	player.command_config = default_command_config();

	return player;
}

Actor create_default_npc(unsigned int id)
{
	Actor actor = {0};
	//memset(&actor, 0, sizeof(Actor));
	//memset(&(actor.model), 0, sizeof(ActorModel));
	actor.actor_state = create_actor_state(id, VEC3(0, 0, -5), VEC3_Z_UP);
	actor.id = id;
	actor.command_config = default_command_config();
	actor.model = BG_MALLOC(ActorModel, 1);
	actor.model = B_load_model_from_file("assets/monkey/monkey.gltf");
	return actor;
}

void my_rotate(vec3 forward, vec3 up, mat4 result)
{
	vec3 right;
	glm_vec3_cross(forward, up, right);
	glm_vec3_normalize(right);
	mat4 rotation = { { right[0],		up[0], 		forward[0], 	0 },
			  { right[1],		up[1], 		forward[1],	0 },
			  { right[2],		up[2],		forward[2],	0 },
			  { 0,			0,		0,		1 } };
	glm_mat4_copy(rotation, result);
}

void set_actor_action(Actor *actor, int action)
{
	actor->model->current_animation = actor->animations[action];
	actor->model->current_animation->time_reference = SDL_GetTicks64()/10.0f;
	actor->model->current_animation->current_time = 0.0f;
}

void update_actor_model(ActorModel *model, ActorState actor_state)
{
	glm_mat4_copy(model->original_position, model->world_space);

	glm_translate(model->world_space, actor_state.position);
	if (memcmp(actor_state.command_state.camera_rotation, GLM_MAT4_ZERO, sizeof(mat4)) != 0)
	{
		glm_mat4_mul(model->world_space, actor_state.command_state.camera_rotation, model->world_space);
	}
	if (model->parent != NULL)
	{
		glm_mat4_mul(model->parent->world_space, model->local_space, model->world_space);
	}
	
	for (int i = 0; i < model->num_children; ++i)
	{
		update_actor_model(model->children[i], actor_state);
	}
}

void B_draw_actors(Actor *all_actors, B_Shader shader, unsigned int num_actors, Renderer renderer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, renderer.g_buffer);
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		if (USE_ALT_CAMERA)
		{
			B_draw_actor_model(all_actors[i].model, renderer.alt_camera, shader);
		}
		else
		{
			B_draw_actor_model(all_actors[i].model, renderer.camera, shader);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void free_actor(Actor actor)
{
	B_free_model(actor.model);
	for (int i = 0; i < actor.num_animations; ++i)
	{
		free_animation(actor.animations[i]);
	}
	BG_FREE(actor.animations);
}

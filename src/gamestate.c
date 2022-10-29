#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "gamestate.h"
#include "actor.h"

#define GRAPHICAL_RENDER 1

GameState create_game_state(void)
{
	GameState state;
	memset(&state, 0, sizeof(GameState));
	state.all_actor_states = NULL;
	state.num_actors = 0;
	state.running = 1;
	return state;
}

void update_game_state(GameState *state)
{
	for (ActorNode *current = state->all_actor_states->first; current != NULL; current = current->next)
	{
		ActorState actor_state = current->actor_state;
		CommandState command_state = actor_state.command_state;
		if (command_state.quit)
		{
			state->running = 0;
			return;
		}
	}
}

void first_actor(GameState *state, ActorState actor_state)
{
	ActorNode *new_node = (ActorNode *)malloc(sizeof(ActorNode));
	memcpy(&new_node->actor_state, &actor_state, sizeof(ActorState));
	new_node->prev = NULL;
	new_node->next = NULL;
	new_node->first = new_node;
	new_node->last = new_node;
	state->all_actor_states = new_node;
	state->num_actors = 1;
}

void push_actor(GameState *state, ActorState actor_state)
{
	if (state->all_actor_states == NULL)
	{
		first_actor(state, actor_state);
		return;
	}
	ActorNode *new_node = (ActorNode *)malloc(sizeof(ActorNode));
	memcpy(&new_node->actor_state, &actor_state, sizeof(ActorState));
	new_node->prev = state->all_actor_states->last;
	new_node->next = NULL;
	state->all_actor_states->last->next = new_node;
	state->num_actors++;

	ActorNode *current = state->all_actor_states->first;
	while (current != NULL)
	{
		current->last = new_node;
		current = current->next;
	}
}

void pop_actor(GameState *state)
{
	ActorNode *current = state->all_actor_states->first;
	ActorNode *last = state->all_actor_states->last;
	while (current != NULL)
	{
		current->last = current->last->prev;
		current = current->next;
	}
	BG_FREE(last);
	state->num_actors--;
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

ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing)
{
	ActorState state;
	memset(&state, 0, sizeof(ActorState));
	memset(&state.command_state, 0, sizeof(CommandState));
	glm_vec3_copy(position, state.position);
	glm_vec3_copy(facing, state.facing);
	state.id = id;
	return state;
}

void free_gamestate(GameState state)
{
	int total = state.num_actors;
	for (int i = 0; i < total; ++i)
	{
		pop_actor(&state);
	}
}
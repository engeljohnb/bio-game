#include <stdio.h>
#include <stdlib.h>
#include "graphics.h"
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

void update_actor_state(ActorState *actor_state, CommandState command_state, float delta_t)
{
	memcpy(&(actor_state->command_state), &command_state, sizeof(CommandState));	
	actor_state->id = command_state.id;
	if (command_state.movement & M_FORWARD)
	{
		vec3 forward;
		glm_vec3_copy(actor_state->front, forward);
		glm_vec3_normalize(forward);
		glm_vec3_add(forward, actor_state->move_direction, actor_state->move_direction);
		glm_vec3_normalize(actor_state->move_direction);
	}
	if (command_state.movement & M_BACKWARD)
	{
		vec3 backward = {0.0, 0.0, 0.0};
		glm_vec3_negate_to(actor_state->front, backward);
		glm_normalize(backward);
		glm_vec3_add(backward, actor_state->move_direction, actor_state->move_direction);
		glm_vec3_normalize(actor_state->move_direction);
	}
	if (command_state.movement & M_LEFT)
	{
		vec3 left = {0.0, 0.0, 0.0};
		glm_vec3_cross(actor_state->front, actor_state->up, left);
		glm_vec3_negate(left);
		glm_vec3_normalize(left);
		glm_vec3_add(left, actor_state->move_direction, actor_state->move_direction);
		glm_vec3_normalize(actor_state->move_direction);
	}
	if (command_state.movement & M_RIGHT)
	{
		vec3 right = {0.0, 0.0, 0.0};
		glm_vec3_cross(actor_state->front, actor_state->up, right);
		glm_normalize(right);
		glm_vec3_add(right, actor_state->move_direction, actor_state->move_direction);
		glm_vec3_normalize(actor_state->move_direction);
	}


	if (command_state.movement)
	{
		actor_state->speed += 0.0001*delta_t;
	}
	else	
	{
		glm_vec3_copy(VEC3_ZERO, actor_state->move_direction);
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
	glm_vec3_scale(actor_state->move_direction, actor_state->speed, velocity);
	glm_vec3_add(actor_state->position, velocity, actor_state->position);
	actor_state->num_updates++;
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
	memcpy(&(new_node->actor_state), &actor_state, sizeof(ActorState));
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

void delete_actor(GameState *state, unsigned int id)
{
	for (ActorNode *current = state->all_actor_states->first; current != NULL; current = current->next)
	{
		ActorState actor_state = current->actor_state;
		if (actor_state.id == id)
		{
			if (actor_state.id == state->all_actor_states->last->actor_state.id)
			{
				pop_actor(state);
				return;
			}
			current->prev->next = current->next;
			if (actor_state.id == state->all_actor_states->first->actor_state.id)
			{
				for (ActorNode *_current = state->all_actor_states->first; _current != NULL; _current = _current->next)
				{
					_current->first = _current->first->next;
				}
			}
			state->num_actors--;
			return;
		}
	}
}

ActorState *get_actor_state_from_id(GameState *state, unsigned int id)
{
	for (ActorNode *current = state->all_actor_states->first; current != NULL; current = current->next)
	{
		if (current->actor_state.id == id)
		{
			return &(current->actor_state);
		}
	}
	fprintf(stderr, "Error: no actor with ID %i: %s %i\n", id, __FILE__, __LINE__);
	exit(0);
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
	glm_vec3_copy(VEC3_ZERO, state.move_direction);
	state.speed = 0;
	state.active = 1;
	state.max_speed = 0.3;
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

#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__
#include <cglm/cglm.h>
#include "input.h"
#define MAX_PLAYERS 4
typedef struct
{
	vec3			position;
	vec3			prev_position;
	vec3			front;
	vec3			move_direction;
	vec3			up;
	CommandState		command_state;
	int			active;
	unsigned int		id;
	int			num_updates;
	float			speed;
	float			max_speed;
} ActorState;

typedef struct
{
	ActorState	actor_states[MAX_PLAYERS];
	unsigned int	num_actors;
	unsigned int	my_id;
} NewPlayerPackage;

typedef struct ActorNode
{
	ActorState		actor_state;
	struct ActorNode	*first;
	struct ActorNode	*last;
	struct ActorNode	*next;
	struct ActorNode	*prev;
} ActorNode;

typedef struct
{
	ActorNode	*all_actor_states;
	int 		num_actors;
	int		running;
} GameState;


GameState create_game_state(void);
ActorState create_actor_state(unsigned int id, vec3 position, vec3 facing);
void update_game_state(GameState *state);
void first_actor(GameState *state, ActorState actor_state);
//void render_gamestate(GameState state, Renderer renderer);
ActorState *get_actor_state_from_id(GameState *state, unsigned int id);
void update_actor_state(ActorState *actor_state, CommandState command_state, float delta_t);
void delete_actor(GameState *state, unsigned int id);
void push_actor(GameState *state, ActorState actor_state);
void pop_actor(GameState *state);
void free_gamestate(GameState state);

#endif

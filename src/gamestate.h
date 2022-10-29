#include "actor.h"
#include "input.h"
#include "graphics.h"

typedef struct
{
	vec3			position;
	vec3			facing;
	CommandState		command_state;
	unsigned int		id;
	/*float			speed;
	float			*forces;
	int			num_forces;*/
} ActorState;
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
void render_game(Actor *all_actors, unsigned int num_actors, Renderer renderer);
void push_actor(GameState *state, ActorState actor_state);
void pop_actor(GameState *state);
void free_gamestate(GameState state);

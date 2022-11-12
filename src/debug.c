#include <stdio.h>
#include "gamestate.h"
#include "debug.h"

void log_actor_state(ActorState actor_state)
{
	if (!actor_state.position[0] &&
	    !actor_state.position[1] &&
	    !actor_state.position[2])
	{
		return;
	}
	fprintf(stdout, "POSITION:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.position[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.position[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.position[2]);
	fprintf(stdout, "PREV_POSITION:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.prev_position[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.prev_position[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.prev_position[2]);
	fprintf(stdout, "FRONT:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.front[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.front[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.front[2]);
	fprintf(stdout, "UP:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.up[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.up[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.up[2]);
	fprintf(stdout, "MOVE_DIRECTION:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.move_direction[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.move_direction[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.move_direction[2]);
	fprintf(stdout, "COMMAND_STATE:\n");
	fprintf(stdout, "----movement: %u\n", actor_state.command_state.movement);
	fprintf(stdout, "----id: %u\n", actor_state.command_state.id);
	fprintf(stdout, "ID:\n");
	fprintf(stdout, "---- %i\n", actor_state.id);
	fprintf(stdout, "SPEED:\n");
	fprintf(stdout, "---- %f\n", actor_state.speed);
	fprintf(stdout, "----------------------------------------\n\n");
}

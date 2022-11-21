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
#include "actor_state.h"
#include "debug.h"
#define DEBUG 1
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
	fprintf(stdout, "FRONT:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.front[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.front[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.front[2]);
	fprintf(stdout, "COMMAND_STATE:\n");
	fprintf(stdout, "----movement: %u\n", actor_state.command_state.movement);
	fprintf(stdout, "----id: %u\n", actor_state.command_state.id);
	fprintf(stdout, "ID:\n");
	fprintf(stdout, "---- %i\n", actor_state.id);
	fprintf(stdout, "SPEED:\n");
	fprintf(stdout, "---- %f\n", actor_state.speed);
	fprintf(stdout, "----------------------------------------\n\n");
}

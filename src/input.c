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
#include <cglm/cglm.h>
#include "input.h"

CommandConfig default_command_config(void)
{
	CommandConfig config = {0};
	config.alt_quit = SDLK_ESCAPE;
	config.left = SDLK_a;
	config.right = SDLK_d;
	config.forward = SDLK_w;
	config.backward = SDLK_s;
	config.x_inverted = 1;
	config.y_inverted = 1;
	return config;
}

/* Sets a command state based on user input. */
int B_update_command_state_ui(CommandState *command_state, CommandConfig config, vec3 move_direction)
{
	static int first_time_thru = 0;
	if (!first_time_thru)
	{
		command_state->look_x = 270;
		command_state->look_y = 0;
		first_time_thru++;
	}
	glm_vec3_copy(move_direction, command_state->move_direction);
	SDL_Event event;
	command_state->look_x_increment = 0;
	command_state->look_y_increment = 0;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				command_state->quit = 1;
				break;
			case SDL_KEYDOWN:
			{
				int key = event.key.keysym.sym;
				if (key == config.alt_quit)
				{
					command_state->quit = 1;
				}
				else if (key == config.forward)
				{
					command_state->movement |= M_FORWARD;
				}
				else if (key == config.backward)
				{
					command_state->movement |= M_BACKWARD;
				}
				else if (key == config.left)
				{
					command_state->movement |= M_LEFT;
				}
				else if (key == config.right)
				{
					command_state->movement |= M_RIGHT;
				}
				break;
			}
			case SDL_KEYUP:
			{
				int key = event.key.keysym.sym;
				if (key == config.alt_quit)
				{
					command_state->quit = 1;
				}
				else if (key == config.forward)
				{
					command_state->movement &= ~M_FORWARD;
				}
				else if (key == config.backward)
				{
					command_state->movement &= ~M_BACKWARD;
				}
				else if (key == config.left)
				{
					command_state->movement &= ~M_LEFT;
				}
				else if (key == config.right)
				{
					command_state->movement &= ~M_RIGHT;
				}
				break;
			}
			case SDL_MOUSEMOTION:
			{
				command_state->look_x_increment = (float)event.motion.xrel*0.4;
				command_state->look_y_increment = -(float)event.motion.yrel*0.4;
				command_state->look_x += (float)event.motion.xrel*0.4;
				command_state->look_y -= (float)event.motion.yrel*0.4;
				break;
			}
			default:
				break;

		}
	}
	return 0;

}

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
#include <glad/glad.h>
#include "input.h"
#include "utils.h"

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
	config.toggle_anti_aliasing = SDLK_t;
	return config;
}

/* Sets a command state based on user input. */
int B_update_command_state_ui(CommandState *command_state, CommandConfig config)
{
	SDL_Event event;
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
				else if (key == config.toggle_anti_aliasing)
				{
					command_state->toggle_anti_aliasing = !command_state->toggle_anti_aliasing;
					if (command_state->toggle_anti_aliasing)
					{
						glEnable(GL_MULTISAMPLE);
					}
					else
					{
						glDisable(GL_MULTISAMPLE);
					}
				}
				//DEBUG
				else if (key == SDLK_p)
				{
					command_state->mode = MODE_SHOW_POSITION;
				}
				else if (key == SDLK_n)
				{
					command_state->mode = MODE_SHOW_NORMALS;
				}
				else if (key == SDLK_i)
				{
					command_state->mode = MODE_SHOW_LIGHTING;
				}
				else if (key == SDLK_c)
				{
					command_state->mode = MODE_SHOW_COLOR;
				}
				break;
			}

			case SDL_MOUSEMOTION:
			{
				command_state->look_x += (float)event.motion.xrel*0.04;
				command_state->look_y += (float)event.motion.yrel*0.04;
				get_rotation_matrix(command_state->look_x, command_state->look_y, command_state->camera_rotation);
				break;
			}
			default:
				break;

		}
	}
	return 0;
}

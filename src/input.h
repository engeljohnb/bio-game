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

#ifndef __B_INPUT_H__
#define __B_INPUT_H__
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include "window.h"

enum MOVEMENT_DIRECTION
{
	M_LEFT = 	0x01,
	M_RIGHT = 	0x02,
	M_FORWARD =	0x04,
	M_BACKWARD = 	0x08
};

enum RENDER_MODE
{
	MODE_SHOW_LIGHTING,
	MODE_SHOW_POSITION,
	MODE_SHOW_NORMALS,
	MODE_SHOW_COLOR 
};

typedef struct CommandState
{
	uint8_t		quit;
	uint8_t 	movement;
	int		elevate;
	unsigned int	id;
	float		look_x;
	float		look_y;
	int		toggle_anti_aliasing;
	vec3		move_direction;
	int		mode;	
	mat4		camera_rotation;
	int		wheel_increment;
	int		random_teleport;
} CommandState;


/* Configuration for what keys execute what commands. default_command_config returns the default setting */
typedef struct CommandConfig
{
	int		x_inverted;
	int		y_inverted;
	int		left;
	int		right;
	int		forward;
	int 		backward;
	int		alt_quit;
} CommandConfig;

CommandConfig default_command_config(void);
int B_update_command_state_ui(CommandState *command_state, CommandConfig config);
int query_log_command(void);
int should_print_debug(void);
#endif

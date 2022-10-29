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

#ifndef __B_INPUT_H__
#define __B_INPUT_H__
#include <SDL2/SDL.h>

enum MOVEMENT_DIRECTION
{
	M_LEFT = 	0x01,
	M_RIGHT = 	0x02,
	M_FORWARD =	0x04,
	M_BACKWARD = 	0x08
};

typedef struct
{
	uint8_t		quit;
	uint8_t 	movement;
	float		look_x_increment;
	float		look_y_increment;
} CommandState;


/* Configuration for what keys execute what commands. default_command_config returns the default setting */
typedef struct
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
#endif

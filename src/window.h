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

#ifndef __B_WINDOW_H__
#define __B_WINDOW_H__
#include <SDL2/SDL.h>

typedef struct
{
	SDL_Window 	*sdl_window;
	SDL_GLContext   gl_context;
	int 		width;
	int 		height;
	vec3 		background_color;

} B_Window;

void B_init();
B_Window B_create_window();
B_Window B_free_window(B_Window window);
void B_clear_window(B_Window);
void B_flip_window(B_Window);

#endif

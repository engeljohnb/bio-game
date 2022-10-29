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
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "window.h"

void B_init(void)
{
	SDL_Init(SDL_INIT_EVERYTHING);
}

B_Window B_create_window(void)
{
	B_Window window;
	int window_width = 0;
	int window_height = 0;
	SDL_Window *size_window = SDL_CreateWindow("Get-size", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_GetWindowSize(size_window, &window_width, &window_height);
	SDL_DestroyWindow(size_window);
	SDL_Window *sdl_window = SDL_CreateWindow("Bio-Game", 0, 0, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (sdl_window == NULL)
	{
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}
	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	glViewport(0, 0, window_width, window_height);
	glEnable(GL_DEPTH_TEST);

	window.sdl_window = sdl_window;
	window.gl_context = gl_context;
	window.width = window_width;
	window.height = window_height;
	window.background_color[0] = 0.0f;
	window.background_color[1] = 0.1f;
	window.background_color[2] = 0.3f;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_WarpMouseInWindow(window.sdl_window, window_width/2, window_height/2);
	SDL_ShowCursor(SDL_DISABLE);
	return window;
}

void B_clear_window(B_Window window)
{
	glClearColor(window.background_color[0], window.background_color[1], window.background_color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void B_flip_window(B_Window window)
{
	SDL_GL_SwapWindow(window.sdl_window);
}

void B_free_window(B_Window window)
{
	SDL_GL_DeleteContext(window.gl_context);
	SDL_DestroyWindow(window.sdl_window);
}

void B_quit(void)
{
	SDL_Quit();
}


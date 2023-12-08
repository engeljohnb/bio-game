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

#include <stdio.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "common.h"
#include "terrain.h"
#include "window.h"

int g_window_width = -1;
int g_window_height = -1;

void get_window_size(int *width, int *height)
{
	*width = g_window_width;
	*height = g_window_height;
}

void B_init(void)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	set_terrain_chunk_dimension(3);
	set_view_distance((float)(get_terrain_xz_scale()*4));
}

B_Window B_create_window(void)
{
	B_Window window;
	int window_width = 16;
	int window_height = 9;
	SDL_Window *size_window = SDL_CreateWindow("Get-size", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_GetWindowSize(size_window, &window_width, &window_height);
	SDL_DestroyWindow(size_window);
	SDL_Window *sdl_window = SDL_CreateWindow("Bio-Game", 10, 10, window_width, window_height, SDL_WINDOW_OPENGL);
	if (sdl_window == NULL)
	{
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}
	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	glViewport(0, 0, window_width, window_height);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	window.sdl_window = sdl_window;
	window.gl_context = gl_context;
	window.width = window_width;
	window.height = window_height;
	window.background_color[0] = 0.0f;
	window.background_color[1] = 0.0f;
	window.background_color[2] = 0.0f;

	/* So there's this bug on some machines where if relative mouse mode is enabled, and the user has two windows running the game, 
	 * all the mouse motion from the one window also happens in the other. However, when I implement
	 * relative mouse motion manually, that causes bugs on other machines. If it becomes necessary, I could chase down these bugs
	 * so everything runs ** perfectly **, but for now my solution is to just recommend not running two games 
	 * in two windows on one computer. */
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_WarpMouseInWindow(window.sdl_window, window_width/2, window_height/2);
	SDL_ShowCursor(SDL_DISABLE);

	g_window_width = window.width;
	g_window_height = window.height;
	return window;
}

void B_clear_window(B_Window window)
{
	glClearColor(window.background_color[0], window.background_color[1], window.background_color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
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


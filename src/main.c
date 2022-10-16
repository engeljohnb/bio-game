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
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include "window.h"
#include "graphics.h"
#include "actor.h"
#include "input.h"

// UP NEXT: Take the boilerplatey stuff from B_create_triangle and make it more general.

int B_load_file(const char *filename, char *buff, int size)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error: could not read file %s\n", filename);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	length++;
	int read_size = 0;
	if (length <= size)
	{
		read_size = length;
	}
	else
	{
		read_size = size;
	}
	memset(buff, 0, size);
	fread(buff, read_size, 1, fp);

	fclose(fp);
	return 0;
}

int B_check_shader(unsigned int id, const char *name, int status)
{
	int success = 1;
	char info_log[512] = { 0 };
	if (status == GL_COMPILE_STATUS)
	{
		glGetShaderiv(id, status, &success);
	}
	else
	{
		glGetProgramiv(id, status, &success);
	}
	if (!success && (status == GL_COMPILE_STATUS))
	{
		glGetShaderInfoLog(id, 512, NULL, info_log);
		fprintf(stderr, "Shader compilation failed for shader %s: %s\n", name, info_log);
		return 0;
	}

	else if (!success && (status == GL_LINK_STATUS))
	{
		glGetProgramInfoLog(id, 512, NULL, info_log);
		fprintf(stderr, "Shader linking failed for shader program: %s\n", info_log);
		return 0;
	}
	return 1;
}

unsigned int B_setup_shader(const char *vert_path, const char *frag_path)
{	
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

	char vertex_buffer[65536] = {0};
	B_load_file(vert_path, vertex_buffer, 65536);
	char fragment_buffer[65536] = {0};
	B_load_file(frag_path, fragment_buffer, 65536);
	const char *vertex_source = vertex_buffer;
	const char *fragment_source = fragment_buffer;

	glShaderSource(vertex_id, 1, &vertex_source, NULL);
	glCompileShader(vertex_id);
	B_check_shader(vertex_id, vert_path, GL_COMPILE_STATUS);

	glShaderSource(fragment_id, 1, &fragment_source, NULL);
	glCompileShader(fragment_id);
	B_check_shader(fragment_id, frag_path, GL_COMPILE_STATUS);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);
	return program_id;
}

void game_loop(B_Window window)
{
	int running = 1;
	Actor player = create_player();
	B_Model triangle = B_create_triangle();
	B_Shader shader = B_setup_shader("src/vertex_shader.vs", "src/fragment_shader.fs");
	while (running)
	{
		B_update_command_state_ui(&player.command_state, player.command_config);
		if (player.command_state.quit)
		{
			running = 0;
		}
		B_clear_window(window);
		B_blit_model(triangle, shader);
		B_flip_window(window);
		SDL_Delay(10);
	}
	B_free_model(triangle);
}

/* Just sets up and dives right into the main loop */
/* All functions and types that contain platform-specific elements are prefixed with B */
int main(void)
{
	B_init();
	B_Window window = B_create_window();
	game_loop(window);	
	B_free_window(window);
	return 0;
}

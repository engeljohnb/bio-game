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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include "server.h"
#include "client.h"
#include "gamestate.h"
#include "window.h"
#include "camera.h"
#include "graphics.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "utils.h"

// UP NEXT: make the cameras third person. Remember: The server (and therefore the game logic) has no reason to worry about what the camera's doing. 

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

Renderer create_default_renderer(B_Window window)
{
	Camera camera = create_camera(window, VEC3(0.0, 0.0, 5.0), VEC3_Z_DOWN, VEC3_Y_UP);
	PointLight point_light = create_point_light(VEC3(4.0, 4.0, 0.0), VEC3(1.0, 1.0, 1.0),1.0);
	B_Shader shader = B_setup_shader("src/vertex_shader.vs", "src/fragment_shader.fs");

	Renderer renderer;
	renderer.camera = camera;
	renderer.window = window;
	renderer.shader = shader;
	renderer.point_light = point_light;
	return renderer;
}

void game_loop(B_Window window)
{
	GameState state = create_game_state();
	unsigned int num_actors = 0;
	// Player
	push_actor(&state, create_actor_state(num_actors++, VEC3(0, 0, -10), VEC3(0, 0, 1)));
	// Monkey
	push_actor(&state, create_actor_state(num_actors++, VEC3(0, 0, 0), VEC3(0, 0, -1)));

	Actor *all_actors = malloc(sizeof(Actor) * 2);
	memset(all_actors, 0, sizeof(Actor) * 2);
	all_actors[0] = create_player(0);
	all_actors[1] = create_default_npc(1);

	Renderer renderer = create_default_renderer(window);
	float frame_time = 0;
	float delta_t = 15.0;
	Actor player = all_actors[0];
	int child_pid = fork();
	if (!child_pid)
	{
		Message message;
		memset(&message, 0, sizeof(Message));
		listen_for_message(&message);
		exit(0);
	}
	else
	{
		send_message(TEMP_SERVER_NAME);
		waitpid(-1, NULL, 0);
	}
	while (state.running)
	{
		frame_time += B_get_frame_time();
		while (frame_time >= delta_t)
		{
			B_update_command_state_ui(&(state.all_actor_states->first->actor_state.command_state), player.command_config);
			update_game_state(&state);
			update_camera(&renderer.camera, state.all_actor_states->first->actor_state.command_state, delta_t);
			frame_time -= delta_t;
		}
		render_game(all_actors, num_actors, renderer);
	}
	free_gamestate(state);
}

/* Just sets up and dives right into the main loop 
 * All functions and types that contain platform-specific elements are prefixed with B */
int main(void)
{
	B_init();
	B_Window window = B_create_window();
	game_loop(window);	
	B_free_window(window);
	B_quit();
	return 0;
}

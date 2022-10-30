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
#include "network.h"
#include "gamestate.h"
#include "window.h"
#include "camera.h"
#include "graphics.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "utils.h"

// UP NEXT: run the game on the desktop. Is it the server? If yes, then run the listening process in the background.
// 	run the game on the laptop. Is it the server? If no, then send a message to the server.
//
//	Then just dive right in and try to get the client-server structure working in earnest.
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
	Camera camera = create_camera(window, VEC3(0.0, 0.0, 0.0), VEC3_Z_DOWN, VEC3_Y_UP);
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
	push_actor(&state, create_actor_state(num_actors++, VEC3(0, 0, -10), VEC3_Z_UP));
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

	while (state.running)
	{
		frame_time += B_get_frame_time();
		while (frame_time >= delta_t)
		{
			B_update_command_state_ui(&(state.all_actor_states->first->actor_state.command_state), player.command_config);
			update_game_state(&state);
			//update_camera(&renderer.camera, state.all_actor_states->first->actor_state.command_state, delta_t);
			frame_time -= delta_t;
		}
		render_game(all_actors, num_actors, renderer);
	}
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		free_actor(all_actors[i]);
	}
	BG_FREE(all_actors);
	free_gamestate(state);
}

void client_main(const char *server_name, B_Window window)
{
	Message message;
	memset(&message, 0, sizeof(Message));
	Actor *all_actors = malloc(sizeof(Actor)*2);
	memset(all_actors, 0, sizeof(Actor)*2);
	unsigned int num_actors = 0;	
	all_actors[0] = create_player(num_actors++);

	float frame_time = 0;
	float delta_t = 15.0;
	Actor player = all_actors[0];
	CommandState command_state;
	memset(&command_state, 0, sizeof(CommandState));

	B_send_message(server_name, JOIN_REQUEST, NULL, 0);
	B_listen_for_message(&message, BLOCKING);
	unsigned int *id = (unsigned int*)message.data;
	command_state.id = *id;
	free_message(message);

	Renderer renderer = create_default_renderer(window);
	int running = 1;
	while (running)
	{
		frame_time += B_get_frame_time();
		while (frame_time >= delta_t)
		{
			B_update_command_state_ui(&command_state, player.command_config);
			B_send_message(server_name, COMMAND_STATE, &command_state, sizeof(CommandState));
			if (B_listen_for_message(&message, NON_BLOCKING) >= 0)
			{
				ActorState *actor_state = (ActorState *)message.data;
				update_actor(&(all_actors[0]), *actor_state);
				update_camera(&renderer.camera, *actor_state);
				if (command_state.quit)
				{
					running = 0;
				}

				free_message(message);
			}
			else
			{
				update_actor_state(&(all_actors[0].actor_state), command_state, delta_t);
				update_actor(&(all_actors[0]), all_actors[0].actor_state);
				update_camera(&renderer.camera, all_actors[0].actor_state);
				if (command_state.quit)
				{
					running = 0;
				}
			}
			//fprintf(stderr, "%f %f %f\n", all_actors[0].actor_state.position[0], all_actors[0].actor_state.position[1], all_actors[0].actor_state.position[2]);
			frame_time -= delta_t;
		}
		render_game(all_actors, num_actors, renderer); 
	}
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		free_actor(all_actors[i]);
	}
	BG_FREE(all_actors);
}

void server_main(void)
{
	GameState state = create_game_state();
	int actor_ids[MAX_PLAYERS];

	unsigned int num_actors = 0;
	float frame_time = 0;
	float delta_t = 15.0;
	while (state.running)
	{
		int progress_state = 0;
		Message message;
		memset(&message, 0, sizeof(Message));
		CommandState *command_state = NULL;
		if ((B_listen_for_message(&message, BLOCKING)) < 0)
		{
			continue;
		}

		switch (message.type)
		{
			case (JOIN_REQUEST):
			{
				push_actor(&state, create_actor_state(num_actors, VEC3(0, 0, 0), VEC3(0, 0, 1)));
				B_send_message(message.from_name, ID_ASSIGNMENT, &num_actors, sizeof(unsigned int));
				num_actors++;
				break;
			}
			case (COMMAND_STATE):
			{
				command_state = (CommandState *)message.data;
				progress_state = 1;
				if (command_state->quit)
				{
					state.running = 0;
				}
			}
			default:
				break;

		}
		frame_time += B_get_frame_time();
		while (frame_time >= delta_t)
		{
			if (progress_state)
			{
				ActorState *actor_state = get_actor_state_from_id(&state, command_state->id);
				update_actor_state(actor_state, *command_state, delta_t);
				B_send_message(message.from_name, ACTOR_STATE, actor_state, sizeof(ActorState));
			}
			frame_time -= delta_t;
		}
		free_message(message);
	}
	free_gamestate(state);
}

/* Just sets up and dives right into the main loop 
 * All functions and types that contain platform-specific elements are prefixed with B */
int main(int argc, char **argv)
{
	
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [SERVER-HOSTNAME]\n", argv[0]);
		return 0;
	}

	char hostname[512] = {0};
	gethostname(hostname, 512);
	if (strncmp(argv[1], hostname, 512) == 0)
	{
		if (!fork())
		{
			B_init();
			B_Window window = B_create_window();
			client_main(argv[1], window);
			B_free_window(window);
			B_quit();
		}
		else
		{
			server_main();
		}
	}
	else
	{
		B_init();
		B_Window window = B_create_window();
		client_main(argv[1], window);
		B_free_window(window);
		B_quit();
		
	}

	//game_loop(window);	
	return 0;
}


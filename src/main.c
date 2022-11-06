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
#include <arpa/inet.h>
#include "network.h"
#include "gamestate.h"
#include "window.h"
#include "camera.h"
#include "graphics.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "utils.h"


/* UP NEXT: Maybe the port number needs to be different for the client and the server?" */
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

void server_loop(const char *port)
{
	B_Connection connections[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&connections[i], 0, sizeof(B_Connection));
	}

	GameState state = create_game_state();
	int num_players = 0;
	B_Connection server_connection = B_connect_to(NULL, port, SETUP_SERVER);
	while (state.running)
	{
		B_Message message;
		memset(&message, 0, sizeof(B_Message));
		B_listen_for_message(server_connection, &message, 0);
		switch (message.type)
		{
			case BORING_TYPE:
			{
				state.running = 0;
				break;
			}
			case JOIN_REQUEST:
			{
				char f_addr[INET_ADDRSTRLEN] = {0};
				char c_addr[INET_ADDRSTRLEN] = {0};
				struct sockaddr_in *from_addr = (struct sockaddr_in *)&message.from_addr;
				struct sockaddr_in *con_addr = (struct sockaddr_in *)&message.from_addr;
				inet_ntop(AF_INET, &(from_addr->sin_addr), f_addr, INET_ADDRSTRLEN);
				inet_ntop(AF_INET, &(con_addr->sin_addr), c_addr, INET_ADDRSTRLEN);
				fprintf(stderr, "%s\n%s\n\n", f_addr, c_addr);
				/*char hostname[512] = {0};
				if (B_get_sender_name(message, hostname, 512) >= 0)
				{*/
					
					//connections[num_players] = B_connect_to(NULL, (char *)message.data, CONNECT_TO_SERVER);
					B_send_reply(server_connection, message, ID_ASSIGNMENT, &num_players, sizeof(int));
				//}
				//B_send_message(server_connection, ID_ASSIGNMENT, &num_players, sizeof(int));
				num_players++;
				break;
			}
			case COMMAND_STATE:
			{
				fprintf(stderr, "Command State\n");
				break;
			}
			case ACTOR_STATE:
			{
				fprintf(stderr, "Actor State\n");
				break;
			}
			case ID_ASSIGNMENT:
			{
				fprintf(stderr, "ID Assignment\n");
				break;
			}
			case NEW_PLAYER:
			{
				fprintf(stderr, "New Payer\n");
				break;
			}
			case ACKNOWLEDGE_NP:
			{
				fprintf(stderr, "Acknowledge NP\n");
				break;
			}
			default:
			{
				fprintf(stderr, "Warning: improper message received\n");
				break;
			}
		}
		free_message(message);
	}
	
}

unsigned int confirm_join_request(B_Connection connection)
{
	B_Message message;
	memset(&message, 0, sizeof(B_Message));
	while (message.type != ID_ASSIGNMENT)
	{
		B_listen_for_message(connection, &message, BLOCKING);
	}
	unsigned int id = *(unsigned int *)message.data;
	free_message(message);
	return id;
}

void game_loop(const char *server_name, const char *port)
{
//	GameState state = create_game_state();
	B_Connection server_connection = B_connect_to(server_name, port, CONNECT_TO_SERVER);
// 	B_Window window = B_create_window();	
	Actor all_actors[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&all_actors[i], 0, sizeof(Actor));
	}

	B_send_message(server_connection, JOIN_REQUEST, NULL, 0);
	unsigned int player_id = confirm_join_request(server_connection);
	fprintf(stderr, "%u\n", player_id);
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
	char port[] = "4590";
	//if (strncmp(argv[1], hostname, 512) == 0)
	if (strncmp(argv[1], "0", 1) == 0)
	{
		//if (!fork())
		//{
			server_loop(port);
		//}
		//else
	//	{
	//		game_loop(NULL, port);
	//	}
	}
	else
	{
		game_loop(NULL, port);
	}
	return 0;
}
/*

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
	ActorState *actor_state = NULL;
	memset(&command_state, 0, sizeof(CommandState));

	B_Connection server_connection = B_connect_to(server_name, "3940");
	B_send_message(server_connection, JOIN_REQUEST, NULL, 0);
	B_listen_for_message(server_connection, &message, BLOCKING);
	unsigned int *id = (unsigned int*)message.data;
	command_state.id = *id;
	free_message(message);

	Renderer renderer = create_default_renderer(window);
	int running = 1;
	while (running)
	{
		int got_message = 0;
		int got_actor_state = 0;
		B_update_command_state_ui(&command_state, player.command_config);
		B_send_message(server_connection, COMMAND_STATE, &command_state, sizeof(CommandState));
		actor_state = &(all_actors[0].actor_state);
		frame_time += B_get_frame_time();
		if (B_listen_for_message(server_connection, &message, NON_BLOCKING) >= 0)
		{
			fprintf(stderr, "Any message at all received\n");
			switch (message.type)
			{
				case (ACTOR_STATE):
				{
					if (actor_state->id == command_state.id)
					{

						actor_state = (ActorState *)message.data;	
					}
					got_actor_state++;
					break;
				}
				case (NEW_PLAYER):
				{
					B_send_message(server_connection, ACKNOWLEDGE_NP, "MESSAGE", sizeof("MESSAGE"));
					fprintf(stderr, "%lu\n", sizeof(Actor));
					break;
				}
				default:
					break;
			}
			got_message++;
		}

		while (frame_time >= delta_t)
		{
			if (!got_actor_state)
			{
				update_actor_state(actor_state, command_state, delta_t);
			}
			update_actor(&(all_actors[0]), *actor_state);
			update_camera(&renderer.camera, *actor_state);
			if (command_state.quit)
			{
				running = 0;
			}
			frame_time -= delta_t;
		}
		render_game(all_actors, num_actors, renderer); 
		if (got_message)
		{
			free_message(message);
		}
	}
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		free_actor(all_actors[i]);
	}
	B_close_connection(server_connection);
	BG_FREE(all_actors);
}

void server_main(void)
{
	GameState state = create_game_state();
	char player_hostnames[MAX_PLAYERS][256];
	B_Connection connections[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(player_hostnames[i], 0, 256);
		memset(&connections[i], 0, sizeof(B_Connection));
	}

	unsigned int num_actors = 0;
	float frame_time = 0;
	float delta_t = 15.0;
	int np_acknowledged = 1;
	B_Connection recv_connection = B_setup_recv_connection("3940");
	while (state.running)
	{
		int progress_state = 0;
		Message message;
		memset(&message, 0, sizeof(Message));
		CommandState *command_state = NULL;
		if ((B_listen_for_message(recv_connection, &message, BLOCKING)) < 0)
		{
			continue;
		}

		if (!np_acknowledged)
		{
			for (unsigned int i = 0; i < num_actors-1; ++i)
			{
				B_send_message(connections[i], NEW_PLAYER, "MESSAGE", strlen("MESSAGE"));
			}
		}
		switch (message.type)
		{
			case (JOIN_REQUEST):
			{
				push_actor(&state, create_actor_state(num_actors, VEC3(0, 0, 0), VEC3(0, 0, 1)));
				B_Connection connection = B_connect_to(message.from_name, "3940");
				B_send_message(connection, ID_ASSIGNMENT, &num_actors, sizeof(unsigned int));
				memcpy(player_hostnames[num_actors], message.from_name, message.from_name_len);
				memcpy(&connections[num_actors], &connection, sizeof(B_Connection));
				if (num_actors)
				{
					np_acknowledged = 0;
					for (unsigned int i = 0; i < num_actors; ++i)
					{
						fprintf(stderr, "%s\n", player_hostnames[i]);
					}
					//B_send_message(player_hostnames[0], NEW_PLAYER, "MESSAGE", strlen("MESSAGE"));
				}
				num_actors++;
				break;
			}
			case (NEW_PLAYER):
			{
				fprintf(stderr, "If this executes, we've got a funny problem\n");
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
				break;
			}
			case (ACKNOWLEDGE_NP):
			{
				np_acknowledged = 1;
				break;
			}
			default:
				break;

		}
		frame_time += B_get_frame_time();
		ActorState *actor_state = get_actor_state_from_id(&state, 0);
		while (frame_time >= delta_t)
		{
			if (progress_state)
			{
				for (unsigned int i = 0; i < num_actors; ++i)
				{
					if (i == command_state->id)
					{
						update_actor_state(actor_state, *command_state, delta_t);
					}
				}
			}
			frame_time -= delta_t;
		}
		for (unsigned int i = 0; i < num_actors; ++i)
		{
			if (strncpy(player_hostnames[i], message.from_name, 256) == 0)
			{
				B_send_message(connections[i], ACTOR_STATE, actor_state, sizeof(ActorState));
				break;
			}
		}
		free_message(message);
	}
	free_gamestate(state);
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		B_close_connection(connections[i]);
	}
}*/

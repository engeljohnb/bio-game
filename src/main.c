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
#include "actor_state.h"
#include "window.h"
#include "camera.h"
#include "graphics.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "utils.h"
#include "debug.h"
// UP NEXT: 
// 		It's so close, just futz with it and I think it'll be perfect.
void server_loop(const char *port)
{
	B_Address addresses[MAX_PLAYERS];
	ActorState players[MAX_PLAYERS];
	CommandState command_states[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&addresses[i], 0, sizeof(B_Address));
		memset(&players[i], 0, sizeof(ActorState));
		memset(&command_states[i], 0, sizeof(CommandState));
	}

	unsigned int num_players = 0;
	float frame_time = 0.0;
	float delta_t = 15.0;
	B_Connection server_connection = B_connect_to(NULL, port, SETUP_SERVER);
	int running = 1;

	while (running)
	{
		B_Message message;
		unsigned int num_states = 0;
		memset(&message, 0, sizeof(B_Message));
		while ((num_states < num_players) || (!num_states))
		{
			int message_return = B_listen_for_message(server_connection, &message, NON_BLOCKING);
			if (message_return <= 0)
			{
				SDL_Delay(10);
				continue;
			}
			switch (message.type)
			{
				case JOIN_REQUEST:
				{
					if (num_players < MAX_PLAYERS)
					{
						addresses[num_players] = B_get_address_from_message(message);
						NewPlayerPackage package;
						memset(&package, 0, sizeof(NewPlayerPackage));
						for (unsigned int i = 0; i < num_players; ++i)
						{
							package.actor_states[i] = players[i];
							package.my_id = num_players;
							package.num_actors = num_players;
						}
						B_send_to_address(server_connection, addresses[num_players], ID_ASSIGNMENT, &package, sizeof(NewPlayerPackage));
						players[num_players] = create_actor_state(num_players, VEC3_ZERO, VEC3_Z_UP);
						for (unsigned int i = 0; i < num_players; ++i)
						{
							B_send_to_address(server_connection, addresses[i], NEW_PLAYER, &num_players, sizeof(unsigned int));
						}
						num_players++;
					}
					break;
				}
				case COMMAND_STATE:
				{
					CommandState command_state = *(CommandState *)message.data;
					if (memcmp(&message.from_addr, &(addresses[command_state.id]), sizeof(B_Address)) == 0)
					{
						command_states[command_state.id] = command_state;
					}
					if (command_state.quit)
					{
						players[command_state.id].active = 0;
					}
					int quit = 1;
					for (unsigned int i = 0; i < num_players; ++i)
					{
						if (players[i].active)
						{
							quit = 0;
							break;
						}
					}
					if (quit && num_players)
					{
						running = 0;
					}
					num_states++;
					break;
				}
				default:
				{
					break;
				}
			}
			for (unsigned int i = 0; i < num_players; ++i)
			{
				if (!players[i].active)
				{
					num_states++;
				}
			}
		}
		for (unsigned int i = 0; i < num_players; ++i)
		{
			update_actor_state_direction(&players[i], &command_states[i]);
		}
		frame_time += B_get_frame_time(delta_t);
		while (frame_time >= delta_t)
		{
			for (unsigned int i = 0; i < num_players; ++i)
			{
				update_actor_state(&players[i], command_states[i], delta_t);
			}
			frame_time -= delta_t;
		}
		for (unsigned int i = 0; i < num_players; ++i)
		{
			for (unsigned int j = 0; j < num_players; ++j)
			{
				B_send_to_address(server_connection, addresses[i], ACTOR_STATE, &(players[j]), sizeof(ActorState));
			}
		}
		free_message(message);
	}
	B_close_connection(server_connection);
}

NewPlayerPackage *confirm_join_request(B_Connection connection)
{
	B_Message message;
	memset(&message, 0, sizeof(B_Message));
	NewPlayerPackage *package = malloc(sizeof(NewPlayerPackage));
	while (message.type != ID_ASSIGNMENT)
	{
		B_listen_for_message(connection, &message, NON_BLOCKING);
		if (message.type != ID_ASSIGNMENT)
		{
			free_message(message);
		}
	}
	memcpy(package, message.data, sizeof(NewPlayerPackage));
	free_message(message);
	return package;
}

void game_loop(const char *server_name, const char *port)
{
	B_Connection server_connection = B_connect_to(server_name, port, CONNECT_TO_SERVER);
 	B_Window window = B_create_window();	
	Renderer renderer = create_default_renderer(window);
	Actor all_actors[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&all_actors[i], 0, sizeof(Actor));
	}
	B_send_message(server_connection, JOIN_REQUEST, NULL, 0);
	NewPlayerPackage *package = confirm_join_request(server_connection);
	unsigned int player_id = package->my_id;
	for (unsigned int i = 0; i < package->num_actors; ++i)
	{
		all_actors[i] = create_player(package->actor_states[i].id);
		all_actors[i].actor_state = package->actor_states[i];
	}
	BG_FREE(package);
	all_actors[player_id] = create_player(player_id);
	unsigned int num_players = player_id+1;
	CommandState command_state = {0};
	command_state.id = player_id;
	int running = 1;
	while (running)
	{
		unsigned int num_states = 0;
		B_Message message;
		B_update_command_state_ui(&command_state, all_actors[player_id].command_config, renderer.camera.front);
		if (command_state.quit)
		{
			running = 0;
		}
		B_send_message(server_connection, COMMAND_STATE, &command_state, sizeof(CommandState));
		while (num_states < num_players)
		{
			int message_return = B_listen_for_message(server_connection, &message, NON_BLOCKING);
			if (message_return <= 0)
			{
				SDL_Delay(10);
				continue;
			}
			switch (message.type)
			{
				case ACTOR_STATE:
				{
					ActorState actor_state = *(ActorState *)message.data;
					all_actors[actor_state.id].actor_state = actor_state;
					num_states++;
					break;
				}
				case NEW_PLAYER:
				{
					unsigned int new_id = *(unsigned int *)message.data;
					all_actors[new_id] = create_player(new_id);
					num_players = new_id + 1;
					break;
				}
				default:
					break;
			}
		}
		for (unsigned int i = 0; i < num_players; ++i)
		{
			update_actor(&all_actors[i], all_actors[i].actor_state);
		}
		update_camera(&renderer.camera, all_actors[player_id].actor_state, command_state.euler);
		render_game(all_actors, num_players, renderer);
		free_message(message);
	}
	for (unsigned int i = 0; i < num_players; ++i)
	{
		free_actor(all_actors[i]);
	}
	B_close_connection(server_connection);
	B_free_window(window);
}

/* Just sets up and dives right into the main loop 
 * All functions and types that contain platform-specific elements are prefixed with B */
int main(int argc, char **argv)
{
	
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s [SERVER-ADDRESS]\n", argv[0]);
		return 0;
	}
	char hostname[512] = {0};
	gethostname(hostname, 512);
	char port[] = "4886";
	if (strncmp(argv[2], "0", 1) == 0)
	{
		server_loop(port);
	}
	else
	{
		B_init();
		game_loop(argv[1], port);
		B_quit();
	}
	return 0;
}

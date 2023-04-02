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
#include "actor_state.h"
#include "window.h"
#include "camera.h"
#include "actor_rendering.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "terrain.h"
#include "asset_loading.h"
#include "terrain_collisions.h"
#include "utils.h"

// Frustum culling for the terrain blocks isn't working.

#define PLAYER_START_POS VEC3(TERRAIN_XZ_SCALE*2, 0, TERRAIN_XZ_SCALE*2)

void game_loop(void)
{	
 	B_Window window = B_create_window();	
	Renderer renderer = create_default_renderer(window);

	// Environment init
	TerrainChunk terrain_chunk = create_terrain_chunk(renderer.g_buffer);
	TerrainElementMesh grass;
	vec2 grass_patch_offsets[9] = {{0.0f}};
	get_grass_patch_offsets(PLAYER_TERRAIN_INDEX_START, grass_patch_offsets);
	grass = create_grass_blade(renderer.g_buffer, terrain_chunk.heightmap_texture);

	// Player init
	Actor all_actors[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&all_actors[i], 0, sizeof(Actor));
	}

	unsigned int player_id = 0;
	all_actors[player_id] = create_player(player_id);
	snap_to_ground(all_actors[player_id].actor_state.position, &terrain_chunk);

	unsigned int num_players = player_id+1;

	// Compile shaders
	B_Shader terrain_shader = B_compile_terrain_shader("src/terrain_shader.vert",
							   "src/terrain_shader.frag",
							   "src/terrain_shader.geo",
							   "src/terrain_shader.ctess",
							   "src/terrain_shader.etess");
	B_Shader actor_shader = B_compile_simple_shader("src/actor_shader.vert",
					                "src/actor_shader.frag");
	B_Shader lighting_shader = B_compile_simple_shader("src/lighting_shader.vert",
					          	   "src/lighting_shader.frag");

	float delta_t = 15.0;
	float frame_time = 0;
	int running = 1;
	int frames = 0;

	while (running)
	{
		// Input update
		B_update_command_state_ui(&all_actors[player_id].actor_state.command_state, all_actors[player_id].command_config);
		if (all_actors[player_id].actor_state.command_state.quit)
		{
			running = 0;
		}


		// Simulation updates
		frame_time += B_get_frame_time();

		for (unsigned int i = 0; i < num_players; ++i)
		{
			update_actor_state_direction(&all_actors[i].actor_state, &all_actors[i].actor_state.command_state);
		}

		all_actors[player_id].actor_state.prev_terrain_index = all_actors[player_id].actor_state.current_terrain_index;
		while (frame_time >= delta_t)
		{
			for (unsigned int i = 0; i < num_players; ++i)
			{
				update_actor_state_position(&all_actors[i].actor_state, all_actors[i].actor_state.command_state, delta_t);
			}
			frame_time -= delta_t;
		}

		for (unsigned int i = 0; i < num_players; ++i)
		{
			if (all_actors[i].actor_state.current_terrain_index != all_actors[i].actor_state.prev_terrain_index)
			{
				get_grass_patch_offsets(all_actors[i].actor_state.current_terrain_index, grass_patch_offsets);
				B_update_terrain_chunk(&terrain_chunk, all_actors[i].actor_state.current_terrain_index);
			}
			update_actor_gravity(&all_actors[i].actor_state, &terrain_chunk, delta_t);
		}

		for (unsigned int i = 0; i < num_players; ++i)
		{
			update_actor_model(all_actors[i].model, all_actors[i].actor_state);
		}
		update_camera(&renderer.camera, all_actors[player_id].actor_state, &terrain_chunk, all_actors[player_id].actor_state.command_state.camera_rotation);


		/* Render */
		mat4 projection_view;
		glm_mat4_mul(renderer.camera.projection_space, renderer.camera.view_space, projection_view);

		int window_width = 0;
		int window_height = 0;
		unsigned int terrain_index = all_actors[player_id].actor_state.current_terrain_index;
		get_window_size(&window_width, &window_height);

		glViewport(0, 0, window_width/2, window_height/2);

		draw_terrain_chunk(&terrain_chunk, terrain_shader, projection_view, all_actors[player_id].actor_state.current_terrain_index);
		B_draw_actors(all_actors, actor_shader, num_players, renderer);
		draw_grass_patches(grass, 
				   projection_view,
				   all_actors[player_id].actor_state.position, 
				   renderer.camera.front,
				   terrain_index, 
				   grass_patch_offsets);

		PointLight point_light;
		memset(&point_light, 0, sizeof(PointLight));
		/* Positions of lights and actors are scaled by 0.01 during the lighting pass, so coordinates of lights should be multiplied by 100
		 * before sending to the GPU. */
		glm_vec3_add(all_actors[player_id].actor_state.position, VEC3(0.0, 100.0, -30.0), point_light.position);
		glm_vec3_copy(VEC3(0.8f, 0.2f, 0.1f), point_light.color);
		point_light.intensity = 2.0f;

		glViewport(0, 0, window_width, window_height);

		B_render_lighting(renderer, lighting_shader, point_light, all_actors[player_id].actor_state.command_state.mode);
		B_flip_window(renderer.window);

		frames++;
	}

	for (unsigned int i = 0; i < num_players; ++i)
	{
		free_actor(all_actors[i]);
	}

	free_terrain_chunk(&terrain_chunk);
	B_free_terrain_element_mesh(grass);
	B_free_window(window);
	free_renderer(renderer);
	B_free_shader(terrain_shader);
	B_free_shader(actor_shader);
	B_free_shader(lighting_shader);
}

/* Just sets up and dives right into the main loop 
 * All functions and types that contain platform-specific elements are prefixed with B */
int main(void)
{
	B_init();
	game_loop();
	B_quit();
	return 0;
}

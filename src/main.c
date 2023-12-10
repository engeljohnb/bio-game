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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include <arpa/inet.h>
#include "actor_state.h"
#include "environment.h"
#include "window.h"
#include "camera.h"
#include "actor_rendering.h"
#include "actor.h"
#include "input.h"
#include "time.h"
#include "terrain.h"
#include "asset_loading.h"
#include "terrain_collisions.h"
#include "grass.h"
#include "utils.h"

#define PLAYER_START_POS VEC3(get_terrain_xz_scale()*2, 0, get_terrain_xz_scale()*2)


// UP NEXT:
// 	Continue optimizing. Is it time to learn about multithreading?
// 	Add pause button
// 	Implement trees

void check_actor_collisions_ice(ActorState *actor_state, EnvironmentCondition environment_condition, float actor_height)
{
	float half_height = actor_height/2.0f;
	if ((environment_condition.temperature < 32) &&
	    (environment_condition.precipitation >= 0.2))
	{
		/* If the actor goes from above the ice to below it, put them on top of the ice */
		if ((actor_state->prev_position[1] >= SEA_LEVEL) &&
		    (actor_state->position[1] < (SEA_LEVEL + half_height)))
		{
			actor_state->position[1] = (SEA_LEVEL + half_height);
		}

		/* If the actor goes from below the ice to above, keep them below the ice */
		if ((actor_state->prev_position[1] <= SEA_LEVEL) &&
		    (actor_state->position[1] > (SEA_LEVEL - half_height)))
		{
			vec3 direction;
			glm_vec3_sub(actor_state->prev_position,
				     actor_state->position,
				     direction);
			glm_vec3_scale(direction, 1.1f, direction);
			glm_vec3_add(direction,
				     actor_state->position, 
				     actor_state->position);
		}
	}
}
void create_grass_patches(Plant grass_patches[9], B_Framebuffer g_buffer, B_Texture heightmap_texture, uint64_t terrain_index)
{
	vec2 grass_patch_offsets[9];
	get_grass_patch_offsets(terrain_index, grass_patch_offsets);
	for (int i = 0; i < 9; ++i)
	{
		grass_patches[i] = create_grass_patch(grass_patch_offsets[i], g_buffer, heightmap_texture);
	}
}

void game_loop(void)
{	
 	B_Window window = B_create_window();	
	Renderer renderer = create_default_renderer(window);

	// Environment init
	TerrainChunk terrain_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_LAND, PLAYER_TERRAIN_INDEX_START);
	Plant grass_patches[9];
	memset(grass_patches, 0, sizeof(Plant)*9);
	create_grass_patches(grass_patches, renderer.g_buffer, terrain_chunk.heightmap_texture, PLAYER_TERRAIN_INDEX_START);

	TerrainChunk water_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_WATER, PLAYER_TERRAIN_INDEX_START);

	ParticleMesh rain_mesh = create_raindrop_mesh(renderer.g_buffer);
	ParticleMesh snow_mesh = create_snowflake_mesh(renderer.g_buffer);

	// Player init
	Actor all_actors[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		memset(&all_actors[i], 0, sizeof(Actor));
	}

	unsigned int player_id = 0;
	all_actors[player_id] = create_player(player_id);
	snap_to_ground(all_actors[player_id].actor_state.position, &terrain_chunk);

	unsigned int num_actors = player_id+1;

	// Compile shaders
	B_Shader terrain_shader = B_compile_terrain_shader("render_progs/terrain_shader.vert",
							   "render_progs/terrain_shader.frag",
							   "render_progs/terrain_shader.geo",
							   "render_progs/terrain_shader.ctess",
							   "render_progs/terrain_shader.etess");
	B_Shader water_shader = B_compile_terrain_shader("render_progs/terrain_shader.vert",
							 "render_progs/water_shader.frag",
							 "render_progs/water_shader.geo",
							 "render_progs/terrain_shader.ctess",
							 "render_progs/water_shader.etess");
	B_Shader actor_shader = B_compile_simple_shader("render_progs/actor_shader.vert",
					                "render_progs/actor_shader.frag");
	B_Shader lighting_shader = B_compile_simple_shader("render_progs/lighting_shader.vert",
					          	   "render_progs/lighting_shader.frag");

	float delta_t = 15.0;
	float frame_time = 0;
	int running = 1;
	int frames = 0;
	int underwater_view = 0;

	vec3 position;
	glm_vec3_sub(all_actors[player_id].actor_state.position, VEC3(-200.0f, -20.0f, 0.0f), position);

	FILE *rain_log = fopen("rain_time_log.txt", "w");

	while (running)
	{
		// Input update
		B_update_command_state_ui(&all_actors[player_id].actor_state.command_state, all_actors[player_id].command_config);
		if (all_actors[player_id].actor_state.command_state.increase_view_distance)
		{
			all_actors[player_id].actor_state.command_state.increase_view_distance = 0;

			set_terrain_chunk_dimension(get_terrain_chunk_dimension()+2);
			int half_dimension = get_terrain_chunk_dimension()/2;
			set_view_distance((get_terrain_xz_scale()*4)*half_dimension);

			free_renderer(renderer);
			renderer = create_default_renderer(window);

			free_terrain_chunk(&terrain_chunk);
			terrain_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_LAND, all_actors[player_id].actor_state.current_terrain_index);

			free_terrain_chunk(&water_chunk);
			water_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_WATER, all_actors[player_id].actor_state.current_terrain_index);

			for (int i = 0; i < 9; ++i)
			{
				free_plant(grass_patches[i]);
			}
			create_grass_patches(grass_patches, renderer.g_buffer, terrain_chunk.heightmap_texture, all_actors[player_id].actor_state.current_terrain_index);
			update_grass_patches(grass_patches, all_actors[player_id].actor_state.current_terrain_index);

		}
		if (all_actors[player_id].actor_state.command_state.decrease_view_distance)
		{
			all_actors[player_id].actor_state.command_state.decrease_view_distance = 0;

			if (get_terrain_chunk_dimension() > 3)
			{
				set_terrain_chunk_dimension(get_terrain_chunk_dimension()-2);
				int half_dimension = get_terrain_chunk_dimension()/2;
				set_view_distance((get_terrain_xz_scale()*4) * half_dimension);

				free_renderer(renderer);
				renderer = create_default_renderer(window);

				free_terrain_chunk(&terrain_chunk);
				terrain_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_LAND, all_actors[player_id].actor_state.current_terrain_index);

				free_terrain_chunk(&water_chunk);
				water_chunk = create_terrain_chunk(renderer.g_buffer, TERRAIN_CHUNK_WATER, all_actors[player_id].actor_state.current_terrain_index);

				for (int i = 0; i < 9; ++i)
				{
					free_plant(grass_patches[i]);
				}
				create_grass_patches(grass_patches, renderer.g_buffer, terrain_chunk.heightmap_texture, all_actors[player_id].actor_state.current_terrain_index);
				update_grass_patches(grass_patches, all_actors[player_id].actor_state.current_terrain_index);

			}
		}

		if (all_actors[player_id].actor_state.command_state.quit)
		{
			running = 0;
		}


		// Simulation updates
		EnvironmentCondition environment_condition = get_environment_condition(all_actors[player_id].actor_state.current_terrain_index);

		frame_time += B_get_frame_time();

		for (unsigned int i = 0; i < num_actors; ++i)
		{	
			update_actor_state_direction(&all_actors[i].actor_state, &all_actors[i].actor_state.command_state);
		}

		all_actors[player_id].actor_state.prev_terrain_index = all_actors[player_id].actor_state.current_terrain_index;
		while (frame_time >= delta_t)
		{
			for (unsigned int i = 0; i < num_actors; ++i)
			{
				update_actor_state_position(&all_actors[i].actor_state, all_actors[i].actor_state.command_state, delta_t);
				check_actor_collisions_ice(&all_actors[i].actor_state, environment_condition, all_actors[i].model->height);
				glm_vec3_copy(all_actors[player_id].actor_state.position, all_actors[player_id].actor_state.prev_position);
				//DEBUG
				if (all_actors[i].actor_state.command_state.random_teleport)
				{
					randomly_teleport_actor(&all_actors[i].actor_state);
				}
			}
			frame_time -= delta_t;
		}

		// TODO: Does this need to be done for all actors, or just the player?
		for (unsigned int i = 0; i < num_actors; ++i)
		{
			if (all_actors[i].actor_state.current_terrain_index != all_actors[i].actor_state.prev_terrain_index)
			{
				update_grass_patches(grass_patches, all_actors[i].actor_state.current_terrain_index);
				B_update_terrain_chunk(&terrain_chunk, all_actors[i].actor_state.current_terrain_index);
				B_update_terrain_chunk(&water_chunk, all_actors[i].actor_state.current_terrain_index);
			}
			update_actor_gravity(&all_actors[i].actor_state, all_actors[i].model->height, &terrain_chunk, delta_t);
			if (should_print_debug())
			{
				print_vec3(all_actors[i].actor_state.position);
			}
		}

		for (unsigned int i = 0; i < num_actors; ++i)
		{
			update_actor_model(all_actors[i].model, all_actors[i].actor_state);
		}
		update_camera(&renderer.camera, all_actors[player_id].actor_state, &terrain_chunk, all_actors[player_id].actor_state.command_state.camera_rotation);

		if (should_print_debug())
		{
			print_temperatures(all_actors[player_id].actor_state.current_terrain_index);
		}

		if (all_actors[player_id].actor_state.command_state.elevate)
		{
			all_actors[player_id].actor_state.position[1] += 3.0;
		}

		/* Render */
		mat4 projection_view;
		glm_mat4_mul(renderer.camera.projection_space, renderer.camera.view_space, projection_view);

		int window_width = 0;
		int window_height = 0;
		uint64_t terrain_index = all_actors[player_id].actor_state.current_terrain_index;
		get_window_size(&window_width, &window_height);

		if (window_height > 1440)
		{
			glViewport(0, 0, window_width/4, window_height/4);
		}
		else
		{
			glViewport(0, 0, window_width/2, window_height/2);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, renderer.g_buffer);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_terrain_chunk(&water_chunk, 
				   water_shader, 
				   projection_view, 
				   all_actors[player_id].actor_state.current_terrain_index,
				   all_actors[player_id].actor_state.front);


		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		draw_terrain_chunk(&terrain_chunk, 
				terrain_shader, 
				projection_view, 
				all_actors[player_id].actor_state.current_terrain_index,
				all_actors[player_id].actor_state.front);

		glDisable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		B_draw_actors(all_actors, actor_shader, num_actors, renderer);

		draw_grass_patches(grass_patches,
				   projection_view,
				   all_actors[player_id].actor_state.position, 
				   renderer.camera.front,
				   terrain_index);

		static float prev_cloudy = 0.0f;	
		if (environment_condition.percent_cloudy > 0.5f)
		{
			if (prev_cloudy <= 0.5f)
			{
				log_rain_time(rain_log);
			}
			float percent_rainy = (environment_condition.percent_cloudy * 2.0f) - 1.0f;
			if (!camera_underwater(all_actors[player_id].actor_state.current_terrain_index))
			{
				if (environment_condition.temperature < 32)
				{
					B_draw_snow(snow_mesh,
						    percent_rainy,
						    projection_view,
						    all_actors[player_id].actor_state.position);
				}

				else
				{
					B_draw_rain(rain_mesh,
						    percent_rainy,
						    projection_view,
						    all_actors[player_id].actor_state.position);

				}
			}
		}
		prev_cloudy = environment_condition.percent_cloudy;


		PointLight player_light;
		memset(&player_light, 0, sizeof(PointLight));
		/* Positions of lights and actors are scaled by 0.01 during the lighting pass, so coordinates of lights should be multiplied by 100
		 * before sending to the GPU. */
		glm_vec3_add(all_actors[player_id].actor_state.position, VEC3(0.0, 100.0, -30.0), player_light.position);
		glm_vec3_copy(VEC3(1.0, 1.0, 1.0), player_light.color);
		player_light.intensity = 2.0f;
		
		glViewport(0, 0, window_width, window_height);

		vec3 sky_color;

		TimeOfDay tod = get_time_of_day();
		DirectionLight weather_light = get_weather_light(environment_condition);
		DirectionLight environment_light = combine_lights(tod.sky_lighting, weather_light, 0.5f);
		
		get_final_sky_color(environment_condition, tod, all_actors[player_id].actor_state.current_terrain_index, sky_color);

		B_render_lighting(renderer, 
				  lighting_shader, 
				  player_light, 
				  environment_light,
				  sky_color, 
				  renderer.camera.position,
				  all_actors[player_id].actor_state.position,
				  environment_condition.percent_cloudy,
				  tod.dew_fog_percent,
				  all_actors[player_id].actor_state.command_state.mode);
		B_flip_window(renderer.window);
		frames++;
	}

	fclose(rain_log);
	for (unsigned int i = 0; i < num_actors; ++i)
	{
		free_actor(all_actors[i]);
	}

	free_terrain_chunk(&terrain_chunk);
	free_terrain_chunk(&water_chunk);
	for (int i = 0; i < 9; ++i)
	{
		free_plant(grass_patches[i]);
	}
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

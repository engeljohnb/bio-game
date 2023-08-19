#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "noise.h"
#include "camera.h"
#include "environment.h"

void B_send_raindrop_mesh_to_gpu(ParticleMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3; 
	int num_vertices = 3;
	mesh->num_elements = 3;

	GLfloat vertices[] = 	
	{ 
		0.5,	1.0, 	0.0,
		0.45,	0.0,	0.0,
		0.55,	0.0,	0.0
	};

	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	int indices[] = { 0, 1, 2 };
	glGenBuffers(1, &mesh->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mesh->num_elements, indices, GL_STATIC_DRAW);

	mesh->num_vertices = num_vertices;
	mesh->shader = B_compile_simple_shader_with_geo("render_progs/rain_shader.vert", "render_progs/rain_shader.geo", "render_progs/rain_shader.frag");
}

void B_send_snowflake_mesh_to_gpu(ParticleMesh *mesh)
{
	size_t stride = sizeof(GLfloat)*3; 
	int num_vertices = 4;
	mesh->num_elements = 6;

	GLfloat vertices[] = { 
		 -1.0f, -1.0f, 0.0f ,
		  1.0f, -1.0f, 0.0f,
		 -1.0f, 1.0f, 0.0f,
		  1.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	int indices[] = {0, 1, 2, 1, 2, 3};
	glGenBuffers(1, &mesh->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mesh->num_elements, indices, GL_DYNAMIC_DRAW);

	mesh->num_vertices = num_vertices;
	mesh->shader = B_compile_simple_shader_with_geo("render_progs/snow_shader.vert", "render_progs/snow_shader.geo", "render_progs/snow_shader.frag");
}

void B_draw_snow(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos)
{
	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);

	static float time = 0.0f;
	int num_instances = 1024;

	mat4 scale = GLM_MAT4_IDENTITY_INIT;
	glm_scale(scale, VEC3(percent_rainy/5.0f, percent_rainy/5.0f, percent_rainy/5.0f));

	B_set_uniform_mat4(mesh.shader, "projection_view", projection_view);
	B_set_uniform_float(mesh.shader, "time", time);
	B_set_uniform_float(mesh.shader, "num_instances", (float)num_instances);
	B_set_uniform_mat4(mesh.shader, "scale", scale);
	B_set_uniform_vec3(mesh.shader, "player_pos", player_pos);
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.num_elements, GL_UNSIGNED_INT, 0, num_instances);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	time += 1.0f;
	glEnable(GL_CULL_FACE);
}

void B_draw_rain(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos)
{
	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);

	static float time = 0.0f;
	int num_instances = 10000 * percent_rainy;

	mat4 scale = GLM_MAT4_IDENTITY_INIT;
	glm_scale(scale, VEC3(percent_rainy, percent_rainy, percent_rainy));

	B_set_uniform_mat4(mesh.shader, "projection_view", projection_view);
	B_set_uniform_float(mesh.shader, "time", time);
	B_set_uniform_float(mesh.shader, "num_instances", (float)num_instances);
	B_set_uniform_mat4(mesh.shader, "scale", scale);
	B_set_uniform_vec3(mesh.shader, "player_pos", player_pos);
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.num_elements, GL_UNSIGNED_INT, 0, num_instances);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	time += 1.0f;
	glEnable(GL_CULL_FACE);
}

ParticleMesh create_raindrop_mesh(int g_buffer)
{
	ParticleMesh mesh;
	memset(&mesh, 0, sizeof(ParticleMesh));
	mesh.g_buffer = g_buffer;
	B_send_raindrop_mesh_to_gpu(&mesh);

	return mesh;
}

ParticleMesh create_snowflake_mesh(int g_buffer)
{
	ParticleMesh mesh;
	memset(&mesh, 0, sizeof(ParticleMesh));
	mesh.g_buffer = g_buffer;
	B_send_snowflake_mesh_to_gpu(&mesh);

	return mesh;
}

float get_current_rain_level(void)
{	
	uint64_t ticks = SDL_GetTicks64();
	float in_game_seconds = (float)((ticks/(uint64_t)1000)%(SECONDS_PER_IN_GAME_DAY));

	float percent = ((1.0f + pnoise1(in_game_seconds/(SECONDS_PER_IN_GAME_DAY), 2))/2.0f) - 0.09f;
	/* Logistic function -- so it's always either rainy (or snowy) or sunny, without too much in-between time*/
	return 1.0f / (1 + pow(2.71828, -10.0f * (percent - 0.5)));
}

float get_current_rain_chances(float current_rain_level, EnvironmentCondition environment_condition)
{	
	return environment_condition.precipitation - current_rain_level;
}

int is_raining(float current_rain_level, float precipitation)
{
	return (current_rain_level <= precipitation);
}

DirectionLight get_weather_light(EnvironmentCondition environment_condition)
{
	vec3 sunny_color;
	vec3 cloudy_color;
	vec3 final_color;
	vec3 direction;
	glm_vec3_copy(VEC3(0.0f, 1.0f, 1.0f), direction);

	glm_vec3_copy(VEC3(1.0f, 0.92f, 0.83f), sunny_color);
	glm_vec3_copy(VEC3(0.8f, 0.8f, 1.0f), cloudy_color);

	glm_vec3_lerp(sunny_color, cloudy_color, environment_condition.percent_cloudy, final_color);
	DirectionLight dir; 
	glm_vec3_copy(direction, dir.direction);
	glm_vec3_copy(final_color, dir.color);
	dir.intensity = (0.8 - environment_condition.percent_cloudy/1.25f);
	return dir;
}

EnvironmentCondition get_environment_condition(uint64_t terrain_index)
{
	uint64_t x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	uint64_t z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)(x_index) / MAX_TERRAIN_BLOCKS;
	float z = (float)(z_index) / MAX_TERRAIN_BLOCKS;

	float precipitation = (1.0f + fbm2d(x*100, z*100, 6, 0.60))/2.0f;
	int temperature = round((1.0f + fbm2d(x, z, 5, 0.80)/2.0f) * 100) - 55.0f;
	/* Logistic function -- because otherwise wayy to much of the map is covered in areas right around 50 degrees.
	 * This creates more polarization in temperatures -- snowy areas and warm areas instead of a bunch of middle ground */
	temperature = 100.0f / (1.0f + powf(2.71828, -0.5f*(temperature-50.0f)));
	float percent_cloudy = get_current_rain_level();

	if (percent_cloudy > 1.0f)
	{
		percent_cloudy = 1.0f;
	}
	else if (percent_cloudy < 0.0f)
	{
		percent_cloudy = 0.0f;
	}

	/*if (percent_cloudy >= 0.5f)
	{
		percent_cloudy += (1.0f-percent_cloudy)/2.0f;
	}
	else if (percent_cloudy < 0.5f)
	{
		percent_cloudy -= (1.0f-percent_cloudy)/2.0f;
	}*/

	if (precipitation < 0.2f)
	{
		percent_cloudy = 0.0f;
	}

	EnvironmentCondition cond = { temperature, precipitation, percent_cloudy};

	return cond;
}

int get_current_tod_phase(double current_time)
{
	if ((current_time >= (B_SUNRISE_TIME * SECONDS_PER_IN_GAME_HOUR)) &&
	    (current_time < (B_MIDDAY_TIME * SECONDS_PER_IN_GAME_HOUR)))
	{
		return B_MORNING;
	}
	
	else if ((current_time >= (B_MIDDAY_TIME * SECONDS_PER_IN_GAME_HOUR)) &&
	         (current_time < (B_SUNSET_TIME * SECONDS_PER_IN_GAME_HOUR)))
	{
		return B_AFTERNOON;
	}
	
	else if ((current_time >= (B_SUNSET_TIME * SECONDS_PER_IN_GAME_HOUR)) &&
	         (current_time < (B_MIDNIGHT_TIME * SECONDS_PER_IN_GAME_HOUR)))
	{
		return B_EVENING;
	}

	else if (current_time < (B_SUNRISE_TIME * SECONDS_PER_IN_GAME_HOUR))
	{
		return B_NIGHT;
	}

	/* IDK */
	else
	{
		fprintf(stderr, "get_current_tod_phase error: Time %f does not fit into any phase slot\n", current_time);
		exit(-1);
	}
}

void get_current_tod_light_direction(int current_phase, double current_time, vec3 dest)
{
	switch (current_phase)
	{
		case B_MORNING:
		{
			float seconds_into_phase = (current_time - B_SUNRISE_TIME)*SECONDS_PER_IN_GAME_HOUR;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 morning_direction;
			vec3 afternoon_direction;

			glm_vec3_copy(MORNING_LIGHT_DIRECTION, morning_direction);
			glm_vec3_copy(AFTERNOON_LIGHT_DIRECTION, afternoon_direction);

			glm_vec3_lerp(morning_direction, afternoon_direction, percent, dest);
			glm_vec3_normalize(dest);
		} break;

		case B_AFTERNOON:
		{
			float seconds_into_phase = (current_time - B_MIDDAY_TIME)*SECONDS_PER_IN_GAME_HOUR;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 afternoon_direction;
			vec3 evening_direction;

			glm_vec3_copy(AFTERNOON_LIGHT_DIRECTION, afternoon_direction);
			glm_vec3_copy(EVENING_LIGHT_DIRECTION, evening_direction);

			glm_vec3_lerp(afternoon_direction, evening_direction, percent, dest);
			glm_vec3_normalize(dest);
		} break;

		case B_EVENING:
		{
			float seconds_into_phase = (current_time - B_SUNSET_TIME)*SECONDS_PER_IN_GAME_HOUR;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 evening_direction;
			vec3 night_direction;

			glm_vec3_copy(EVENING_LIGHT_DIRECTION, evening_direction);
			glm_vec3_copy(NIGHT_LIGHT_DIRECTION, night_direction);

			glm_vec3_lerp(evening_direction, night_direction, percent, dest);
			glm_vec3_normalize(dest);
		} break;

		case B_NIGHT:
		{
			float seconds_into_phase = current_time;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 night_direction;
			vec3 morning_direction;

			glm_vec3_copy(NIGHT_LIGHT_DIRECTION, night_direction);
			glm_vec3_copy(MORNING_LIGHT_DIRECTION, morning_direction);

			glm_vec3_lerp(night_direction, morning_direction, percent, dest);
			glm_vec3_normalize(dest);
		} break;

		default:
		{
			fprintf(stderr, "get_current_tod_light_direction error: This definitely should not be executing\n");
			exit(-1);
		} break;
	}
}

void get_current_tod_sky_color(int current_phase, double current_time, vec3 dest)
{
	switch (current_phase)
	{
		case B_MORNING:
		{
			float seconds_into_phase = current_time - (B_SUNRISE_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 morning_color;
			vec3 afternoon_color;

			glm_vec3_copy(MORNING_SKY_COLOR, morning_color);
			glm_vec3_copy(AFTERNOON_SKY_COLOR, afternoon_color);

			glm_vec3_lerp(morning_color, afternoon_color, percent, dest);
		} break;

		case B_AFTERNOON:
		{
			float seconds_into_phase = current_time - (B_MIDDAY_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 afternoon_color;
			vec3 evening_color;

			glm_vec3_copy(AFTERNOON_SKY_COLOR, afternoon_color);
			glm_vec3_copy(EVENING_SKY_COLOR, evening_color);

			glm_vec3_lerp(afternoon_color, evening_color, percent, dest);
		} break;

		case B_EVENING:
		{
			float seconds_into_phase = current_time - (B_SUNSET_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 evening_color;
			vec3 night_color;

			glm_vec3_copy(EVENING_SKY_COLOR, evening_color);
			glm_vec3_copy(NIGHT_SKY_COLOR, night_color);

			glm_vec3_lerp(evening_color, night_color, percent, dest);
		} break;

		case B_NIGHT:
		{
			float seconds_into_phase = current_time;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);
			vec3 night_color;
			vec3 morning_color;

			glm_vec3_copy(NIGHT_SKY_COLOR, night_color);
			glm_vec3_copy(MORNING_SKY_COLOR, morning_color);

			glm_vec3_lerp(night_color, morning_color, percent, dest);
		} break;

		default:
		{
			fprintf(stderr, "get_current_tod_sky_color error: This definitely should not be executing\n");
			exit(-1);
		} break;
	}
}

void get_current_tod_light_color(int current_phase, double current_time, vec3 dest)
{
	switch (current_phase)
	{
		case B_MORNING:
		{
			float seconds_into_phase = current_time - (B_SUNRISE_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 morning_color;
			vec3 afternoon_color;

			glm_vec3_copy(MORNING_LIGHT_COLOR, morning_color);
			glm_vec3_copy(AFTERNOON_LIGHT_COLOR, afternoon_color);

			glm_vec3_lerp(morning_color, afternoon_color, percent, dest);
		} break;

		case B_AFTERNOON:
		{
			float seconds_into_phase = current_time - (B_MIDDAY_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 afternoon_color;
			vec3 evening_color;

			glm_vec3_copy(AFTERNOON_LIGHT_COLOR, afternoon_color);
			glm_vec3_copy(EVENING_LIGHT_COLOR, evening_color);

			glm_vec3_lerp(afternoon_color, evening_color, percent, dest);
		} break;

		case B_EVENING:
		{
			float seconds_into_phase = current_time - (B_SUNSET_TIME*SECONDS_PER_IN_GAME_HOUR);
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);

			vec3 evening_color;
			vec3 night_color;

			glm_vec3_copy(EVENING_LIGHT_COLOR, evening_color);
			glm_vec3_copy(NIGHT_LIGHT_COLOR, night_color);

			glm_vec3_lerp(evening_color, night_color, percent, dest);
		} break;

		case B_NIGHT:
		{
			float seconds_into_phase = current_time;
			float percent = seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);
			vec3 night_color;
			vec3 morning_color;

			glm_vec3_copy(NIGHT_LIGHT_COLOR, night_color);
			glm_vec3_copy(MORNING_LIGHT_COLOR, morning_color);

			glm_vec3_lerp(night_color, morning_color, percent, dest);
		} break;

		default:
		{
			fprintf(stderr, "get_current_tod_sky_color error: This definitely should not be executing\n");
			exit(-1);
		} break;
	}
}
TimeOfDay get_time_of_day(void)
{
	TimeOfDay time_of_day = {0};
	double current_time = B_get_seconds_into_current_phase();
	//current_time += 15.0 * SECONDS_PER_IN_GAME_HOUR;
	time_of_day.current_phase = get_current_tod_phase(current_time);
	get_current_tod_sky_color(time_of_day.current_phase, current_time, time_of_day.sky_color);

	vec3 light_direction; 
	vec3 light_color;
	get_current_tod_light_direction(time_of_day.current_phase, current_time, light_direction);
	get_current_tod_light_color(time_of_day.current_phase, current_time, light_color);
	DirectionLight light = create_direction_light(light_direction, light_color, 0.3);
	time_of_day.sky_lighting = light;

	return time_of_day;
}

void get_final_sky_color(EnvironmentCondition environment_condition, TimeOfDay tod, uint64_t terrain_index, vec3 dest)
{
	if (camera_underwater(terrain_index))
	{
		glm_vec3_copy(UNDERWATER_SKY_COLOR, dest);
	}
	else
	{
		vec3 cloudy_color;
		glm_vec3_copy(VEC3(0.06f, 0.06f, 0.068f), cloudy_color);
		glm_vec3_lerp(tod.sky_color, cloudy_color, environment_condition.percent_cloudy, dest);
	}
}

int camera_underwater(uint64_t terrain_index)
{
	EnvironmentCondition cond = get_environment_condition(terrain_index);
	return ((get_camera_height() < SEA_LEVEL) && (cond.precipitation >= 0.2));
}
void print_temperatures(uint64_t player_terrain_index)
{
	uint64_t x_offset = -1;
	uint64_t z_offset = -MAX_TERRAIN_BLOCKS;
	for (int i = 0; i < 9; ++i)
	{
		uint64_t terrain_index = player_terrain_index + x_offset + z_offset;
		EnvironmentCondition cond = get_environment_condition(terrain_index);
		fprintf(stderr, "%lu %i %f\t", terrain_index, cond.temperature, cond.precipitation);
		if ((i == 2) || (i == 5) || (i == 8))
		{
			fprintf(stderr, "\n");
		}
		x_offset++;
		if (x_offset > 1)
		{
			x_offset = -1;
			z_offset += MAX_TERRAIN_BLOCKS;
		}
	}
	fprintf(stderr, "-----------------------------\n\n");
}


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
	float in_game_milliseconds = (float)((ticks/(uint64_t)100)%(SECONDS_PER_IN_GAME_DAY*14));

	float percent = 0.5f + (noise1(in_game_milliseconds/(SECONDS_PER_IN_GAME_DAY*14))/2.0f);
	/* Logistic function -- so it's always either rainy (or snowy) or sunny, without too much in-between time*/
	float final = 1.0f / (1.0f + pow(2.71828, -25.0f * (percent - 0.6f)));
	//fprintf(stderr, "%f %f\n", percent, final);
	return final;
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
	glm_vec3_copy(VEC3(0.9f, 0.9f, 0.9f), cloudy_color);

	glm_vec3_lerp(sunny_color, cloudy_color, environment_condition.percent_cloudy, final_color);
	DirectionLight dir; 
	glm_vec3_copy(direction, dir.direction);
	glm_vec3_copy(final_color, dir.color);
	TimeOfDay tod = get_time_of_day();
	float intensity = glm_percent(0.3f, 0.8f, tod.sky_lighting.intensity);
	dir.intensity = intensity;
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

	if (precipitation < 0.2f)
	{
		percent_cloudy = 0.0f;
	}

	EnvironmentCondition cond = { temperature, precipitation, percent_cloudy };

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

float get_current_tod_light_intensity(int current_phase, double current_time)
{
	if ((current_phase == B_NIGHT) || (current_phase == B_MORNING))
	{
		float seconds_into_phase = current_time;
		float percent = seconds_into_phase/(12.0f*SECONDS_PER_IN_GAME_HOUR);
		return lerp(0.3f, 1.5f, percent); 
	}
	else if ((current_phase == B_AFTERNOON) || (current_phase == B_EVENING))
	{
		float seconds_into_phase = current_time - (B_MIDDAY_TIME * SECONDS_PER_IN_GAME_HOUR);
		float percent = seconds_into_phase/(12.0f*SECONDS_PER_IN_GAME_HOUR);
		return lerp(1.5f, 0.3f, percent);					
	}

	else
	{
		fprintf(stderr, "get_current_tod_light_intensity error: This definitely should not be executing\n");
		exit(-1);
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
			fprintf(stderr, "get_current_tod_light_color error: This definitely should not be executing\n");
			exit(-1);
		} break;
	}
}

float get_tod_dew_fog_percent(int current_phase, float current_time)
{
	switch (current_phase)
	{
		case B_MORNING:
		{
			float seconds_into_phase = current_time - (B_SUNRISE_TIME*SECONDS_PER_IN_GAME_HOUR);
			return seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR);
		} break;

		case B_AFTERNOON:
		{
			return 0.0f;
		} break;

		case B_EVENING:
		{
			return 0.0f;
		} break;

		case B_NIGHT:
		{
			float seconds_into_phase = current_time;
			return 1.0f - (seconds_into_phase/(6.0f*SECONDS_PER_IN_GAME_HOUR));
		} break;

		default:
		{
			fprintf(stderr, "get_current_tod_light_color error: This definitely should not be executing\n");
			exit(-1);
		} break;
	}

}

TimeOfDay get_time_of_day(void)
{
	TimeOfDay time_of_day = {0};
	double current_time = B_get_seconds_into_current_day();
	time_of_day.current_phase = get_current_tod_phase(current_time);
	get_current_tod_sky_color(time_of_day.current_phase, current_time, time_of_day.sky_color);

	vec3 light_direction; 
	vec3 light_color;
	get_current_tod_light_direction(time_of_day.current_phase, current_time, light_direction);
	get_current_tod_light_color(time_of_day.current_phase, current_time, light_color);
	float light_intensity = get_current_tod_light_intensity(time_of_day.current_phase, current_time);
	DirectionLight light = create_direction_light(light_direction, light_color, light_intensity);
	time_of_day.sky_lighting = light;

	time_of_day.dew_fog_percent = get_tod_dew_fog_percent(time_of_day.current_phase, current_time);

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
		glm_vec3_copy(VEC3(0.06f, 0.16f, 0.168f), cloudy_color);
		glm_vec3_lerp(tod.sky_color, cloudy_color, environment_condition.percent_cloudy, dest);
	}
}

int camera_underwater(uint64_t terrain_index)
{
	EnvironmentCondition cond = get_environment_condition(terrain_index);
	return ((get_camera_height() < SEA_LEVEL) && (cond.precipitation >= 0.2));
}

void tod_phase_to_string(int phase, char *dest)
{
	switch (phase)
	{
		case B_MORNING:
			snprintf(dest, strlen("MORNING")+1, "%s", "MORNING");
			break;
		case B_AFTERNOON:
			snprintf(dest, strlen("AFTERNOON")+1, "%s", "AFTERNOON");
			break;
		case B_EVENING:
			snprintf(dest, strlen("EVENING")+1, "%s", "EVENING");
			break;
		case B_NIGHT:
			snprintf(dest, strlen("NIGHT")+1, "%s", "NIGHT");
			break;
		default:
			fprintf(stderr, "tod_to_string error: tod phase %i not recognized\n", phase);
			break;
	}
}

DirectionLight combine_lights(DirectionLight a, DirectionLight b, float t)
{
	DirectionLight final_light = {0};
	glm_vec3_lerp(a.color, b.color, t, final_light.color);
	glm_vec3_lerp(a.direction, b.direction, t, final_light.direction);
	final_light.intensity = glm_lerp(a.intensity, b.intensity, t);
	return final_light;
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

void log_rain_time(FILE *fp)
{
	double hour = B_get_current_in_game_hour();
	double minute = B_get_current_minute();
	double second = B_get_current_second();

	TimeOfDay tod = get_time_of_day();
	char phase[128] = {0};

	tod_phase_to_string(tod.current_phase, phase);

	float number_of_days = hour/24.0f;

	fprintf(fp, "%f:%f:%f %s %f %lu\n", hour, minute, second, phase, number_of_days, SDL_GetTicks64());
}

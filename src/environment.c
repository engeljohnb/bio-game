#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "noise.h"
#include "environment.h"

float get_current_rain_level(void)
{	
	uint64_t ticks = SDL_GetTicks64();
	float in_game_hour = (float)((ticks/(uint64_t)3600000)%300);
	float in_game_minute = (float)((ticks/(uint64_t)60000)%60);
	float in_game_second = (float)((ticks/(uint64_t)1000)%60);

	// This is here because I felt there wasn't enough sunshine.
	float sunshine_offset = pnoise1(in_game_minute/60.0f, 3);

	float percent = pnoise3(in_game_hour/300.0f, in_game_minute/60.0f, in_game_second/60.0f, 2, 2, 2);
	return (percent/2.0 + 0.5) - sunshine_offset ;
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

EnvironmentCondition get_environment_condition(unsigned int terrain_index)
{
	unsigned int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	unsigned int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)(x_index) / MAX_TERRAIN_BLOCKS;
	float z = (float)(z_index) / MAX_TERRAIN_BLOCKS;

	float precipitation = fbm2d(x*100, z*100, 6, 0.60);
	int temperature = round(fbm2d(x, z, 5, 0.80) * 140);
	float percent_cloudy = get_current_rain_level();

	if (precipitation < 0.2f)
	{
		percent_cloudy = 0.0f;
	}

	if (percent_cloudy > 1.0f)
	{
		percent_cloudy = 1.0f;
	}
	else if (percent_cloudy < 0.0f)
	{
		percent_cloudy = 0.0f;
	}

	EnvironmentCondition cond = { temperature, precipitation, percent_cloudy};

	return cond;
}

void get_sky_color(EnvironmentCondition environment_condition, vec3 dest)
{
	// TODO: Consider getting the average of all 9 terrain blocks so the weather doesn't abruptly change when the
	// player crosses a block border.
	vec3 sunny_color;
	vec3 cloudy_color;

	glm_vec3_copy(VEC3(0.33f, 0.49f, 0.8f), sunny_color);
	glm_vec3_copy(VEC3(0.06f, 0.06f, 0.068f), cloudy_color);

	glm_vec3_lerp(sunny_color, cloudy_color, environment_condition.percent_cloudy, dest);
}

void print_temperatures(unsigned int player_terrain_index)
{
	unsigned int x_offset = -1;
	unsigned int z_offset = -MAX_TERRAIN_BLOCKS;
	for (int i = 0; i < 9; ++i)
	{
		int terrain_index = player_terrain_index + x_offset + z_offset;
		EnvironmentCondition cond = get_environment_condition(terrain_index);
		fprintf(stderr, "%i %i %f\t", terrain_index, cond.temperature, cond.precipitation);
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


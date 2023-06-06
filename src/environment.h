#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__
#include <cglm/cglm.h>
#include "common.h"

typedef struct
{
	/* temperature and precipitation do not change for a given
	 * terrain block */
	int	temperature;
	float	precipitation;
	/* percent_cloudy changes over time */
	float	percent_cloudy;
} EnvironmentCondition;

void get_sky_color(EnvironmentCondition environment_condition, vec3 dest);
EnvironmentCondition get_environment_condition(unsigned int terrain_index);
DirectionLight get_weather_light(EnvironmentCondition environment_condition);
void print_temperatures(unsigned int player_terrain_index);
#endif

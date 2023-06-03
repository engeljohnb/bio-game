#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

typedef struct
{
	int	temperature;
	float	precipitation;
} EnvironmentCondition;

EnvironmentCondition get_environment_condition(unsigned int terrain_index);
void print_temperatures(unsigned int player_terrain_index);
#endif

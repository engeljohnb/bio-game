#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "noise.h"
#include "environment.h"

EnvironmentCondition get_environment_condition(unsigned int terrain_index)
{
	unsigned int x_index = terrain_index % MAX_TERRAIN_BLOCKS;
	unsigned int z_index = terrain_index / MAX_TERRAIN_BLOCKS;

	float x = (float)(x_index) / MAX_TERRAIN_BLOCKS;
	float z = (float)(z_index) / MAX_TERRAIN_BLOCKS;

	float precipitation = fbm2d(x*100, z*100, 6, 0.60);
	int temperature = round(fbm2d(x, z, 5, 0.80) * 140);
	EnvironmentCondition cond = { temperature, precipitation };

	return cond;
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


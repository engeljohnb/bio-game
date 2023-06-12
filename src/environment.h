#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__
#include <cglm/cglm.h>
#include "common.h"

typedef struct EnvironmentCondition
{
	/* temperature and precipitation do not change for a given
	 * terrain block */
	int	temperature;
	float	precipitation;
	/* percent_cloudy changes over time */
	float	percent_cloudy;
} EnvironmentCondition;

typedef struct ParticleMesh
{
	B_Framebuffer	g_buffer;
	unsigned int	vbo;
	unsigned int	vao;
	unsigned int	ebo;
	int 		num_elements;
	int		num_vertices;
	B_Shader	shader;
} ParticleMesh;

void get_sky_color(EnvironmentCondition environment_condition, vec3 dest);
EnvironmentCondition get_environment_condition(unsigned int terrain_index);
DirectionLight get_weather_light(EnvironmentCondition environment_condition);
ParticleMesh create_raindrop_mesh(int g_buffer);
void B_draw_rain(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos,
		 vec3 player_facing);
void print_temperatures(unsigned int player_terrain_index);
#endif

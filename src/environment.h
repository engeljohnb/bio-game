#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__
#include <cglm/cglm.h>
#include "time.h"
#include "common.h"

/* EnvironmentCondition is the weather & lighting conditions that are random at any given time */
typedef struct EnvironmentCondition
{
	/* temperature and precipitation do not change for a given
	 * terrain block */
	int	temperature;
	float	precipitation;
	/* percent_cloudy changes over time */
	float	percent_cloudy;
} EnvironmentCondition;

#define MORNING_LIGHT_DIRECTION VEC3(1.0f, 0.0f, 0.0f)
#define AFTERNOON_LIGHT_DIRECTION VEC3(0.0f, 1.0f, 0.0f)
#define EVENING_LIGHT_DIRECTION VEC3(-1.0f, 0.0f, 0.0f)
#define NIGHT_LIGHT_DIRECTION VEC3(0.0f, -1.0f, 0.0f)

#define MORNING_SKY_COLOR VEC3(0.40f, 0.50f, 0.97f)
#define AFTERNOON_SKY_COLOR VEC3(0.32f, 0.56f, 0.97f)
#define EVENING_SKY_COLOR VEC3(0.32f, 0.23f, 0.32f)
#define NIGHT_SKY_COLOR VEC3(0.0f, 0.0f, 0.0f)
#define UNDERWATER_SKY_COLOR VEC3(0.12, 0.17, 0.42)

#define MORNING_LIGHT_COLOR VEC3(0.32f, 0.32f, 0.32f)
#define AFTERNOON_LIGHT_COLOR VEC3(0.39f, 0.39f, 0.34f)
#define EVENING_LIGHT_COLOR VEC3(0.32f, 0.29f, 0.32f)
#define NIGHT_LIGHT_COLOR VEC3(0.14f, 0.14f, 0.34f)


/* TimeOfDay is the weather & lighting conditions based on
 * the in-game time*/
typedef struct TimeOfDay
{
	int		current_phase;
	vec3		sky_color;
	//vec3		sun_color;
	DirectionLight	sky_lighting;
	//vec3		sun_position;
	float 		dew_fog_percent;
} TimeOfDay;

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

void get_final_sky_color(EnvironmentCondition environment_condition, TimeOfDay tod, uint64_t terrain_index, vec3 dest);
EnvironmentCondition get_environment_condition(uint64_t terrain_index);
DirectionLight get_weather_light(EnvironmentCondition environment_condition);
ParticleMesh create_raindrop_mesh(int g_buffer);
TimeOfDay get_time_of_day(void);

void B_draw_snow(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos);

void B_draw_rain(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos);
ParticleMesh create_snowflake_mesh(int g_buffer);
int camera_underwater(uint64_t terrain_index);
void print_temperatures(uint64_t player_terrain_index);
void log_rain_time(FILE *fp);
void tod_phase_to_string(int phase, char *dest);
DirectionLight combine_lights(DirectionLight a, DirectionLight b, float t);
#endif

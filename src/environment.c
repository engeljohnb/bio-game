#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "noise.h"
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
	mesh->shader = B_compile_simple_shader_with_geo("src/rain_shader.vert", "src/rain_shader.geo", "src/rain_shader.frag");
}
void B_draw_rain(ParticleMesh mesh,
		 float percent_rainy,
		 mat4 projection_view,
		 vec3 player_pos,
		 vec3 player_facing)
{
	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);

	static float time = 0.0f;
	int num_instances = 10000 * percent_rainy;
	vec3 frustum_corners[8];
	get_frustum_corners(projection_view, frustum_corners);

	mat4 scale = GLM_MAT4_IDENTITY_INIT;
	glm_scale(scale, VEC3(percent_rainy, percent_rainy, percent_rainy));

	B_set_uniform_mat4(mesh.shader, "projection_view", projection_view);
	B_set_uniform_float(mesh.shader, "time", time);
	B_set_uniform_float(mesh.shader, "num_instances", (float)num_instances);
	B_set_uniform_mat4(mesh.shader, "scale", scale);
	B_set_uniform_vec3(mesh.shader, "player_pos", player_pos);
	B_set_uniform_vec3(mesh.shader, "player_facing", player_facing);
	for (int i = 0; i < 8; ++i)
	{
		char name[128] = {0};
		snprintf(name, 128, "frustum_corners[%i]", i);
		B_set_uniform_vec3(mesh.shader, name, frustum_corners[i]);
	}
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.num_elements, GL_UNSIGNED_INT, 0, num_instances);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	time += 1.0f;
}

ParticleMesh create_raindrop_mesh(int g_buffer)
{
	ParticleMesh mesh;
	memset(&mesh, 0, sizeof(ParticleMesh));
	mesh.g_buffer = g_buffer;
	B_send_raindrop_mesh_to_gpu(&mesh);

	return mesh;
}

float get_current_rain_level(void)
{	
	uint64_t ticks = SDL_GetTicks64();
	float in_game_seconds = (float)((ticks/(uint64_t)1000)%1000);

	float percent = (1.0f + pnoise1(in_game_seconds/1000.0f, 2)) / 2.0f;

	/* Because I wanted more sunshine */
	percent += noise1(in_game_seconds/1000.0f);
	return 1.0f - glm_clamp((((percent - 0.5f) * 500.0f) + 0.5f)/100.0f, 0.0f, 1.0f);
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

	float precipitation = (1.0f + fbm2d(x*100, z*100, 6, 0.60))/2.0f;
	int temperature = round(fbm2d(x, z, 5, 0.80) * 140);
	if (temperature < 0)
	{
		temperature = 0;
	}
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

// TODO: Why can you still get snow in an area with negative precipitation?
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


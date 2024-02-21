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
#include <string.h>
#include "common.h"

float g_view_distance = 0.0f;
int g_terrain_xz_scale = 300;
int g_print_debug = 0;

int should_print_debug(void)
{
	return g_print_debug;
}

void set_should_print_debug(int i)
{
	g_print_debug = i;
}

int get_terrain_xz_scale(void)
{
	return g_terrain_xz_scale;
}

int set_terrain_xz_scale(int xz_scale)
{
	g_terrain_xz_scale = xz_scale;
	return g_terrain_xz_scale;
}

void set_view_distance(float distance)
{
	g_view_distance = distance;
}

float get_view_distance(void)
{
	return g_view_distance;
}


DirectionLight create_direction_light(vec3 direction, vec3 color, float intensity)
{
	DirectionLight light = {0};
	glm_vec3_copy(direction, light.direction);
	glm_vec3_copy(color, light.color);
	light.intensity = intensity;
	return light;
}

B_Shader B_compile_simple_shader(const char *vert_path, const char *frag_path)
{	
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

	char vertex_buffer[65536] = {0};
	B_load_file(vert_path, vertex_buffer, 65536);
	char fragment_buffer[65536] = {0};
	B_load_file(frag_path, fragment_buffer, 65536);
	const char *vertex_source = vertex_buffer;
	const char *fragment_source = fragment_buffer;

	glShaderSource(vertex_id, 1, &vertex_source, NULL);
	glCompileShader(vertex_id);
	B_check_shader(vertex_id, vert_path, GL_COMPILE_STATUS);

	glShaderSource(fragment_id, 1, &fragment_source, NULL);
	glCompileShader(fragment_id);
	B_check_shader(fragment_id, frag_path, GL_COMPILE_STATUS);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);

	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
	return program_id;
}

B_Shader B_compile_simple_shader_with_geo(const char *vert_path, const char *geo_path, const char *frag_path)
{	
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	unsigned int geo_id = glCreateShader(GL_GEOMETRY_SHADER);

	char vertex_buffer[4096] = {0};
	B_load_file(vert_path, vertex_buffer, 4096);
	char fragment_buffer[4096] = {0};
	B_load_file(frag_path, fragment_buffer, 4096);
	char geo_buffer[4096] = {0};
	B_load_file(geo_path, geo_buffer, 4096);

	const char *vertex_source = vertex_buffer;
	const char *fragment_source = fragment_buffer;
	const char *geo_source = geo_buffer;

	glShaderSource(vertex_id, 1, &vertex_source, NULL);
	glCompileShader(vertex_id);
	B_check_shader(vertex_id, vert_path, GL_COMPILE_STATUS);

	glShaderSource(fragment_id, 1, &fragment_source, NULL);
	glCompileShader(fragment_id);
	B_check_shader(fragment_id, frag_path, GL_COMPILE_STATUS);

	glShaderSource(geo_id, 1, &geo_source, NULL);
	glCompileShader(geo_id);
	B_check_shader(geo_id, geo_path, GL_COMPILE_STATUS);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glAttachShader(program_id, geo_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);

	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
	glDeleteShader(geo_id);
	return program_id;
}

B_Shader B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *geo_path, const char *ctess_path, const char *etess_path)
{
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	unsigned int ctess_id = glCreateShader(GL_TESS_CONTROL_SHADER);
	unsigned int etess_id = glCreateShader(GL_TESS_EVALUATION_SHADER);
	unsigned int geo_id = glCreateShader(GL_GEOMETRY_SHADER);

	char *vertex_buffer = BG_MALLOC(char, 4096);
	char *fragment_buffer = BG_MALLOC(char, 4096);
	char *ctess_buffer = BG_MALLOC(char, 4096);
	char *etess_buffer = BG_MALLOC(char, 8192);
	char *geo_buffer = BG_MALLOC(char, 12188);
	
	B_load_file(vert_path, vertex_buffer, 4096);
	B_load_file(frag_path, fragment_buffer, 4096);
	B_load_file(ctess_path, ctess_buffer, 4096);
	B_load_file(etess_path, etess_buffer, 8192);
	B_load_file(geo_path, geo_buffer, 12188);

	const char *vertex_source = vertex_buffer;
	const char *fragment_source = fragment_buffer;
	const char *ctess_source = ctess_buffer;
	const char *etess_source = etess_buffer;
	const char *geo_source = geo_buffer;

	glShaderSource(vertex_id, 1, &vertex_source, NULL);
	glCompileShader(vertex_id);
	B_check_shader(vertex_id, vert_path, GL_COMPILE_STATUS);

	glShaderSource(fragment_id, 1, &fragment_source, NULL);
	glCompileShader(fragment_id);
	B_check_shader(fragment_id, frag_path, GL_COMPILE_STATUS);

	glShaderSource(ctess_id, 1, &ctess_source, NULL);
	glCompileShader(ctess_id);
	B_check_shader(ctess_id, ctess_path, GL_COMPILE_STATUS);

	glShaderSource(etess_id, 1, &etess_source, NULL);
	glCompileShader(etess_id);
	B_check_shader(etess_id, etess_path, GL_COMPILE_STATUS);

	glShaderSource(geo_id, 1, &geo_source, NULL);
	glCompileShader(geo_id);
	B_check_shader(geo_id, geo_path, GL_COMPILE_STATUS);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glAttachShader(program_id, ctess_id);
	glAttachShader(program_id, etess_id);
	glAttachShader(program_id, geo_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);

	BG_FREE(vertex_buffer);
	BG_FREE(fragment_buffer);
	BG_FREE(ctess_buffer);
	BG_FREE(etess_buffer);
	BG_FREE(geo_buffer);

	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
	glDeleteShader(ctess_id);
	glDeleteShader(etess_id);
	glDeleteShader(geo_id);

	return program_id;
}

void B_free_shader(B_Shader shader)
{
	glDeleteShader(shader);
}

int B_check_shader(unsigned int id, const char *name, int status)
{
	int success = 1;
	char info_log[512] = { 0 };
	if (status == GL_COMPILE_STATUS)
	{
		glGetShaderiv(id, status, &success);
	}
	else
	{
		glGetProgramiv(id, status, &success);
	}
	if (!success && (status == GL_COMPILE_STATUS))
	{
		glGetShaderInfoLog(id, 512, NULL, info_log);
		fprintf(stderr, "Shader compilation failed for shader %s: %s\n", name, info_log);
		return 0;
	}

	else if (!success && (status == GL_LINK_STATUS))
	{
		glGetProgramInfoLog(id, 512, NULL, info_log);
		fprintf(stderr, "Shader linking failed for shader program: %s\n", info_log);
		return 0;
	}
	return 1;
}

void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value)
{
	glUseProgram(shader);
	GLfloat matrix[16] = { 0.0f };
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			matrix[i*4+j] = value[i][j];
		}
	}
	int uniform_location = glGetUniformLocation(shader, name);
	if (uniform_location == -1)
	{
		fprintf(stderr, "Uniform location for %s returned error: %i\n", name,  glGetError());
		exit(-1);
	}
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, matrix);
}

void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value)
{
	glUseProgram(shader);
	glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2], value[3]);
}

void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value)
{
	glUseProgram(shader);
	glUniform3f(glGetUniformLocation(shader, name), value[0], value[1], value[2]);
}

void B_set_uniform_vec2(B_Shader shader, char *name, vec2 value)
{
	glUseProgram(shader);
	glUniform2f(glGetUniformLocation(shader, name), value[0], value[1]);
}

void B_set_uniform_float(B_Shader shader, char *name, float value)
{
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, name), value);
}

void B_set_uniform_int(B_Shader shader, char *name, int value)
{
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, name), value);
}
void B_set_uniform_direction_light(B_Shader shader, char *name, DirectionLight value)
{
	glUseProgram(shader);
	char intensity_name[256] = {0};
	char direction_name[256] = {0};
	char color_name[256] = {0};

	char *intensity = &intensity_name[0];
	char *direction = &direction_name[0];
	char *color = &color_name[0];

	cat_to(name, ".intensity", intensity, 256);
	cat_to(name, ".direction", direction, 256);
	cat_to(name, ".color", color, 256);
	
	glUniform1f(glGetUniformLocation(shader, intensity_name), value.intensity);
	glUniform3f(glGetUniformLocation(shader, direction_name), value.direction[0], value.direction[1], value.direction[2]);
	glUniform3f(glGetUniformLocation(shader, color_name), value.color[0], value.color[1], value.color[2]);
}

void B_set_uniform_point_light(B_Shader shader, char *name, PointLight value)
{
	glUseProgram(shader);
	char intensity_name[256] = {0};
	char position_name[256] = {0};
	char color_name[256] = {0};

	char *intensity = &intensity_name[0];
	char *position = &position_name[0];
	char *color = &color_name[0];

	cat_to(name, ".intensity", intensity, 256);
	cat_to(name, ".position", position, 256);
	cat_to(name, ".color", color, 256);
	
	glUniform1f(glGetUniformLocation(shader, intensity_name), value.intensity);
	glUniform3f(glGetUniformLocation(shader, position_name), value.position[0], value.position[1], value.position[2]);
	glUniform3f(glGetUniformLocation(shader, color_name), value.color[0], value.color[1], value.color[2]);
}

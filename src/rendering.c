#include <stdio.h>
#include <stdlib.h>
#include "rendering.h"

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
	//glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, matrix);
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

/*
    Bio-Game is a game for designing your own microorganism. 
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
#include "actor_state.h"
#include "debug.h"
unsigned int g_vec_draw_vao;
unsigned int g_vec_draw_vbo;
unsigned int g_vec_draw_shader;

unsigned int B_get_vec_draw_vao(void)
{
	return g_vec_draw_vao;
}

unsigned int B_compile_vec_draw_shader(const char *vert_path, const char *frag_path)
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

	glShaderSource(fragment_id, 1, &fragment_source, NULL);
	glCompileShader(fragment_id);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);

	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
	return program_id;
}

void B_send_vec_draw_to_gpu(void)
{
	glGenVertexArrays(1, &g_vec_draw_vao);
	glBindVertexArray(g_vec_draw_vao);
	glGenBuffers(1, &g_vec_draw_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_vec_draw_vbo);
	GLfloat vertex[6] = {0.0f};
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(GLfloat), vertex, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);	

	g_vec_draw_shader = B_compile_vec_draw_shader("src/vec_draw.vert", "src/vec_draw.frag");
}

void draw_vec(vec3 vec, unsigned int g_buffer)
{
	glBindVertexArray(g_vec_draw_vao);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
	glUseProgram(g_vec_draw_shader);
	glUniform3f(glGetUniformLocation(g_vec_draw_shader, "v_vector"), vec[0], vec[1], vec[2]);
	glDrawArrays(GL_LINES, 0, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init_vec_draw(void)
{
	B_send_vec_draw_to_gpu();
}


void log_actor_state(ActorState actor_state)
{
	if (!actor_state.position[0] &&
	    !actor_state.position[1] &&
	    !actor_state.position[2])
	{
		return;
	}
	fprintf(stdout, "POSITION:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.position[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.position[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.position[2]);
	fprintf(stdout, "FRONT:\n");
	fprintf(stdout, "----[0]: %f\n", actor_state.front[0]);
	fprintf(stdout, "----[1]: %f\n", actor_state.front[1]);
	fprintf(stdout, "----[2]: %f\n", actor_state.front[2]);
	fprintf(stdout, "COMMAND_STATE:\n");
	fprintf(stdout, "----movement: %u\n", actor_state.command_state.movement);
	fprintf(stdout, "----id: %u\n", actor_state.command_state.id);
	fprintf(stdout, "ID:\n");
	fprintf(stdout, "---- %i\n", actor_state.id);
	fprintf(stdout, "SPEED:\n");
	fprintf(stdout, "---- %f\n", actor_state.speed);
	fprintf(stdout, "----------------------------------------\n\n");
}

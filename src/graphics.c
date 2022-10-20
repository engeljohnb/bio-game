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

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "window.h"
#include "graphics.h"
#include "utils.h"

void get_triangle_data(B_Vertex *buffer)
{
	B_Vertex vertices[] = { { {-1.0, -1.0, 0.0}, { 0.0, 0.0, -1.0}, { 0.0, 0.0, 0.0} },
		 	      	{ {1.0, -1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} },
			      	{ {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} } };
	buffer = memcpy(buffer, vertices, TRIANGLE_SIZE);
}

B_Model B_create_triangle()
{
	B_Vertex *triangle_data = malloc(TRIANGLE_SIZE);
	memset(triangle_data, 0, TRIANGLE_SIZE);
	get_triangle_data(triangle_data);
	unsigned int faces[] = { 0, 1, 2 };
	B_Model model = B_create_model_from(triangle_data, faces, 3, 3);
	return model;
}

B_Mesh B_create_mesh(B_Vertex *vertices, unsigned int *faces, unsigned int num_vertices, unsigned int num_faces)
{
	B_Mesh mesh;
	memset(&mesh, 0, sizeof(mesh));
	mesh.active = 1;
	mesh.num_vertices = num_vertices;
	mesh.num_faces = num_faces;
	mesh.vertices = malloc(mesh.num_vertices * sizeof(B_Vertex));
	mesh.vertices = memcpy(mesh.vertices, vertices, mesh.num_vertices*sizeof(B_Vertex));
	mesh.faces = malloc(sizeof(unsigned int) * num_faces);
	mesh.faces = memcpy(mesh.faces, faces, mesh.num_faces*sizeof(unsigned int));
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ebo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices*sizeof(B_Vertex), mesh.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_faces*sizeof(unsigned int), mesh.faces, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)sizeof(B_Vertex));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)(sizeof(B_Vertex)*2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	return mesh;
}

B_Model B_create_model(B_Mesh *meshes, unsigned int num_meshes)
{
	B_Model model;
	memset(model.meshes, 0, sizeof(B_Mesh)*MAX_MESHES);
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		model.meshes[i].active = 0;
	}

	unsigned int mesh_count = (unsigned int)maxi(num_meshes, MAX_MESHES);
	for (int i = 0; i < mesh_count; ++i)
	{

		model.meshes[i] = meshes[i];
	}
	glm_mat4_identity(model.local_space);
	glm_mat4_identity(model.world_space);
	return model;
}

B_Model B_create_model_from(B_Vertex *vertices, unsigned int *faces, unsigned int num_vertices, unsigned int num_faces)
{
	B_Model model;
	memset(model.meshes, 0, sizeof(B_Mesh)*MAX_MESHES);
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		model.meshes[i].active = 0;
	}
	B_Mesh mesh = B_create_mesh(vertices, faces, num_vertices, num_faces);
	model.meshes[0] = mesh;
	glm_mat4_identity(model.local_space);
	glm_mat4_identity(model.world_space);
	return model;
}


B_Model load_model_from_file(const char *filename)
{
/*	B_Model model = { 0 };
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error: could not read file %s\n", filename);
		return model;
	}
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	uint8_t buff[length];
	memset(buff, 0, length);
	fread(buff, length, 1, fp);
	fclose(fp);

	const char *original = buff;
	memset(vertices, 0, length/sizeof(float));
	memset(indices, 0, length/sizeof(int));
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		float vertices[length/sizeof(float)];
		int indices[length/sizeof(int)];
		char begin_data_key[256] = {0};
		char end_data_key[256] = {0};
		sprintf(begin_data_key, "MESH %u:\n", i);
		sprintf(end_data_key, "MESH %u:\n", i+1);
		char *begin_data = strstr(buff, begin_data_key);
		if (begin_data == NULL)
		{
			break;
		}
		char *end_data = strstr(buff, end_data_key);
		int num_bytes = 0;
		if (end_data == NULL)
		{
			num_bytes = (original + length) - begin_data;
		}
		else
		{
			num_bytes = end_data - begin_data;
		}

		memcpy(vertices,
	}
*/
}

void B_blit_model(B_Model model, B_Shader shader)
{
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if (model.meshes[i].active)
		{
			glBindVertexArray(model.meshes[i].vao);
			glUseProgram(shader);
			vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
			B_set_uniform_vec4(shader, "color", color);
			B_set_uniform_mat4(shader, "local_space", model.local_space);
			B_set_uniform_mat4(shader, "world_space", model.world_space);
			//glDrawArrays(GL_TRIANGLES, 0, model.meshes[i].num_vertices);
			glDrawElements(GL_TRIANGLES, model.meshes[i].num_faces, GL_UNSIGNED_INT, 0);
		}
	}
}

void B_free_model(B_Model model)
{
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if(model.meshes[i].active)
		{	
			//glDeleteBuffers(1, &model.meshes[i].ebo);
			glDeleteBuffers(1, &model.meshes[i].vbo);
		}
		else
		{
			break;
		}

	}
}

void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value)
{
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, value[0]);
}
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value)
{
	glUseProgram(shader);
	glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2], value[3]);
}

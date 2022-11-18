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
#include <string.h>
#include "actor.h"
#include "camera.h"
#include "window.h"
#include "graphics.h"
#include "utils.h"

PointLight create_point_light(vec3 position, vec3 color, float intensity)
{
	PointLight light = { 0 };
	glm_vec3_copy(position, light.position);
	glm_vec3_copy(color, light.color);
	light.intensity = intensity;
	return light;
}
void get_triangle_data(B_Vertex *buffer)
{
	B_Vertex vertices[] = { { {-1.0, -1.0, 0.0}, { 0.0, 0.0, -1.0}, { 0.0, 0.0, 0.0} },
		 	      	{ {1.0, -1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} },
			      	{ {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} } };
	buffer = memcpy(buffer, vertices, TRIANGLE_SIZE);
}

B_Model B_create_triangle(void)
{
	B_Vertex *triangle_data = (B_Vertex *)malloc(TRIANGLE_SIZE);
	memset(triangle_data, 0, TRIANGLE_SIZE);
	get_triangle_data(triangle_data);
	unsigned int faces[] = { 0, 1, 2 };
	B_Model model = B_create_model_from(triangle_data, faces, 3, 3);
	return model;
}

B_Mesh B_create_mesh(B_Vertex *vertices, unsigned int *faces, unsigned int num_vertices, unsigned int num_faces)
{
	B_Mesh mesh;
	memset(&mesh, 0, sizeof(B_Mesh));
	mesh.active = 1;
	mesh.num_vertices = num_vertices;
	mesh.num_faces = num_faces;
	mesh.vertices = (B_Vertex *)malloc(mesh.num_vertices * sizeof(B_Vertex));
	mesh.vertices = memcpy(mesh.vertices, vertices, mesh.num_vertices*sizeof(B_Vertex));
	mesh.faces = (unsigned int *)malloc(sizeof(unsigned int) * num_faces);
	memset(mesh.faces, 0, sizeof(unsigned int) * num_faces);
	mesh.faces = memcpy(mesh.faces, faces, mesh.num_faces*sizeof(unsigned int));

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices*sizeof(B_Vertex), mesh.vertices, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &mesh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_faces*sizeof(unsigned int), mesh.faces, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3)*3, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3)*3, (void*)(sizeof(vec3)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3)*3, (void*)(sizeof(vec3)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	return mesh;
}

B_Model B_create_model(B_Mesh *meshes, unsigned int num_meshes)
{
	B_Model model;
	memset(model.meshes, 0, sizeof(B_Mesh)*MAX_MESHES);
	model.valid = 1;
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		model.meshes[i].active = 0;
	}

	unsigned int mesh_count = num_meshes;
	for (unsigned int i = 0; i < mesh_count; ++i)
	{

		model.meshes[i] = meshes[i];
	}
	glm_mat4_identity(model.world_space);
	glm_mat4_identity(model.local_space);
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
	glm_mat4_identity(model.world_space);
	glm_mat4_identity(model.local_space);
	return model;
}

B_Model create_cube(void)
{
		float _vertices_[] = {
			    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			
			    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			
			    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			
			     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			
			    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			
			    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f,
			    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f
			};

	B_Model cube = B_create_model_from((B_Vertex *)_vertices_, NULL, 36, 0);
	return cube;
}


B_Model load_model_from_file(const char *filename)
{
	B_Model model;
	memset(&model, -1, sizeof(B_Model));
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error: could not read file %s\n", filename);
		return model;
	}
	fseek(fp, 0L, SEEK_END);
	int total_length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	uint8_t buff[total_length];
	memset(buff, 0, total_length);
	fread(buff, total_length, 1, fp);
	fclose(fp);

	unsigned int num_meshes = 0;
	unsigned int num_faces = 0;
	unsigned int *vertex_sizes = 0;
	unsigned int *face_sizes = 0;
	uint8_t **vertex_data = get_data_after_punctuated(buff, "B_MESH:", "END_MESHES", total_length, &num_meshes, &vertex_sizes);
	uint8_t **face_data = get_data_after_punctuated(buff, "B_FACES:", "END_FACES", total_length, &num_faces, &face_sizes);
	if (num_meshes > MAX_MESHES)
	{
		fprintf(stderr, "WARNING: mesh loaded from %s contains more meshes than maximum supported.\n", filename);
		num_meshes = MAX_MESHES;
	}

	B_Mesh meshes[num_meshes];
	for (unsigned int i = 0; i < num_meshes; ++i)
	{
		meshes[i] = B_create_mesh((B_Vertex *)vertex_data[i], (unsigned int *)face_data[i], vertex_sizes[i] / sizeof(B_Vertex), face_sizes[i]/sizeof(unsigned int));
	}
	model = B_create_model(meshes, num_meshes);
	BG_FREE(vertex_sizes);
	BG_FREE(face_sizes);
	for (unsigned int i = 0; i < num_meshes; ++i)
	{
		BG_FREE(vertex_data[i]);
		BG_FREE(face_data[i]);
	}
	BG_FREE(vertex_data);
	BG_FREE(face_data);
	return model;
}

void B_blit_model(B_Model model, Camera camera, B_Shader shader, PointLight point_light)
{
	if (!model.valid)
	{
		return;
	}
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if (model.meshes[i].active)
		{
			glBindVertexArray(model.meshes[i].vao);
			vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
			B_set_uniform_vec3(shader, "point_lights[0].position", point_light.position);
			B_set_uniform_vec3(shader, "point_lights[0].color", point_light.color);
			B_set_uniform_float(shader, "point_lights[0].intensity", point_light.intensity);
			B_set_uniform_vec4(shader, "color", color);
			B_set_uniform_mat4(shader, "view_space", camera.view_space);
			B_set_uniform_mat4(shader, "world_space", model.world_space);
			B_set_uniform_mat4(shader, "local_space", model.local_space);
			B_set_uniform_mat4(shader, "projection_space", camera.projection_space);
			glUseProgram(shader);
			if (model.meshes[i].num_faces)
			{
				glDrawElements(GL_TRIANGLES, model.meshes[i].num_faces, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, model.meshes[i].num_vertices);
			}
		}
	}
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

unsigned int B_setup_shader(const char *vert_path, const char *frag_path)
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
	return program_id;
}

Renderer create_default_renderer(B_Window window)
{
	Camera camera = create_camera(window, VEC3(0.0, 0.0, 0.0), VEC3_Z_DOWN);
	PointLight point_light = create_point_light(VEC3(4.0, 4.0, 0.0), VEC3(1.0, 1.0, 1.0),1.0);
	B_Shader shader = B_setup_shader("src/vertex_shader.vs", "src/fragment_shader.fs");

	Renderer renderer;
	renderer.camera = camera;
	renderer.window = window;
	renderer.shader = shader;
	renderer.point_light = point_light;
	return renderer;
}


void B_free_model(B_Model model)
{
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if(model.meshes[i].active)
		{	
			glDeleteBuffers(1, &model.meshes[i].ebo);
			glDeleteBuffers(1, &model.meshes[i].vbo);
			BG_FREE(model.meshes[i].vertices);
			if (model.meshes[i].faces)
			{
				BG_FREE(model.meshes[i].faces);
			}
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

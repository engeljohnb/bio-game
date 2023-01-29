#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "utils.h"
#include "tessellation.h"
#include "utils.h"

/*T_Mesh B_create_terrain_mesh(void)
{
	int width = 0;
	int height = 0;
	int num_channels = 0;
	T_Vertex *vertices = generate_t_vertices(width, height);
	uint8_t *texture_data = stbi_load(height_map_file, &width, &height, &num_channels, 0);
	
	T_Mesh mesh = B_send_terrain_mesh_to_gpu(vertices, 20*20, texture_data, width, height, 20);
	BG_FREE(vertices);
	BG_FREE(texture_data);
	return mesh;
}*/

T_Mesh B_create_terrain_mesh(int width, int height)
{
	int num_vertices = width*height*4;
	T_Vertex vertices[num_vertices];
	memset(vertices, 0, sizeof(T_Vertex) * num_vertices);

	for (int i = 0; i < num_vertices; i += 4)
	{
		vertices[i].position[0] = -1.0;
		vertices[i].position[1] = 0.0;
		vertices[i].position[2] = -1.0;

		vertices[i+1].position[0] = -1.0;
		vertices[i+1].position[1] = 0.0;
		vertices[i+1].position[2] = 1.0;

		vertices[i+2].position[0] = 1.0;
		vertices[i+2].position[1] = 0.0;
		vertices[i+2].position[2] = 1.0;

		vertices[i+3].position[0] = 1.0;
		vertices[i+3].position[1] = 0.0;
		vertices[i+3].position[2] = -1.0;
	}

	T_Mesh mesh = B_send_terrain_mesh_to_gpu(vertices, num_vertices, height);
	return mesh;
}

T_Mesh B_send_terrain_mesh_to_gpu(T_Vertex *vertices, int num_vertices, int num_columns)
{
	T_Mesh mesh = {0};
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	size_t stride = sizeof(GLfloat)*3;

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	mesh.num_vertices = num_vertices;
	mesh.num_columns = num_columns;
	return mesh;
}

unsigned int B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *ctess_path, const char *etess_path)
{	
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	unsigned int ctess_id = glCreateShader(GL_TESS_CONTROL_SHADER);
	unsigned int etess_id = glCreateShader(GL_TESS_EVALUATION_SHADER);

	char vertex_buffer[4096] = {0};
	char fragment_buffer[4096] = {0};
	char ctess_buffer[4096] = {0};
	char etess_buffer[8192] = {0};
	
	B_load_file(vert_path, vertex_buffer, 4096);
	B_load_file(frag_path, fragment_buffer, 4096);
	B_load_file(ctess_path, ctess_buffer, 4096);
	B_load_file(etess_path, etess_buffer, 8192);


	const char *vertex_source = vertex_buffer;
	const char *fragment_source = fragment_buffer;
	const char *ctess_source = ctess_buffer;
	const char *etess_source = etess_buffer;

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

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glAttachShader(program_id, ctess_id);
	glAttachShader(program_id, etess_id);
	glLinkProgram(program_id);
	B_check_shader(program_id, "shader program", GL_LINK_STATUS);
	return program_id;
}

void B_draw_terrain(T_Mesh mesh, mat4 world_space, B_Shader shader, Camera *camera)
{
	glUseProgram(shader);
	B_set_uniform_mat4(shader, "world_space", world_space);
	mat4 projection_view;
	glm_mat4_mul(camera->projection_space, camera->view_space, projection_view);
	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_columns);
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
}


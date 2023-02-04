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
	size_t stride = sizeof(GLfloat)*3 + sizeof(GLfloat)*2;

	T_Vertex texture_vertices[6] = { 
		{{ -1.0f, -1.0f, 0.0f} ,   {0.0f, 0.0f}},
		{{ -1.0f, 1.0f, 0.0f},	   {0.0f, 1.0f}},
		{{  1.0f, -1.0f, 0.0f},    {1.0f, 0.0f}},
		{{  1.0f, -1.0f, 0.0f},    {1.0f, 0.0f}},
		{{  -1.0f, 1.0f, 0.0f},    {0.0f, 1.0f}},
		{{  1.0f, 1.0f, 0.0f},     {1.0f, 1.0f}} };

	glGenVertexArrays(1, &mesh.lighting_vao);
	glBindVertexArray(mesh.lighting_vao);
	glGenBuffers(1, &mesh.lighting_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.lighting_vbo);
	glBufferData(GL_ARRAY_BUFFER, 6*stride, texture_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenFramebuffers(1, &mesh.g_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);
	glGenTextures(1, &mesh.normal_texture);
	glBindTexture(GL_TEXTURE_2D, mesh.normal_texture);
	// TODO: B_get_screen_width()
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mesh.normal_texture, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);
	unsigned int depth_buffer;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
                fprintf(stderr, "Not compltet!\n");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Mesh VAO
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*stride, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	mesh.num_vertices = num_vertices;
	mesh.num_columns = num_columns;
	return mesh;
}

unsigned int B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *geo_path, const char *ctess_path, const char *etess_path)
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
	char *geo_buffer = BG_MALLOC(char, 4096);
	
	B_load_file(vert_path, vertex_buffer, 4096);
	B_load_file(frag_path, fragment_buffer, 4096);
	B_load_file(ctess_path, ctess_buffer, 4096);
	B_load_file(etess_path, etess_buffer, 8192);
	B_load_file(geo_path, geo_buffer, 4096);

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

	return program_id;
}

void B_draw_terrain_lighting_pass(T_Mesh mesh, B_Shader shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh.normal_texture);
	B_set_uniform_int(shader, "f_texture", 0);
	GLuint indices[] = { 0, 1, 2, 3, 4, 5 };
	glBindVertexArray(mesh.lighting_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
}

void B_draw_terrain_geo_pass(T_Mesh mesh, B_Shader shader, Camera *camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, mesh.g_buffer);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);
	mat4 projection_view;
	glm_mat4_mul(camera->projection_space, camera->view_space, projection_view);
	B_set_uniform_mat4(shader, "projection_view_space", projection_view);
	B_set_uniform_int(shader, "patches_per_column", mesh.num_columns);
	B_set_uniform_float(shader, "tessellation_level", 8);
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_PATCHES, 0, mesh.num_vertices);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


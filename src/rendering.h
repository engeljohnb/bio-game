#ifndef __RENDER_H__
#define __RENDER_H__

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "camera.h"
#include "window.h"

typedef unsigned int B_Shader;
typedef unsigned int B_Framebuffer;
typedef unsigned int B_Texture;

typedef struct
{
	Camera		camera;
	B_Window	window;
	B_Shader	shader;	
	float		delta_t;
	B_Texture	normal_texture;
	B_Texture	position_texture;
	B_Framebuffer	g_buffer;
	unsigned int	lighting_vao;
	unsigned int	lighting_vbo;
} Renderer;

B_Framebuffer B_generate_g_buffer(B_Texture *normal_texture, B_Texture *position_texture, unsigned int *lighting_vao, unsigned int *lighting_vbo);
void B_render_lighting(Renderer renderer, B_Shader shader);
Renderer create_default_renderer(B_Window window);
unsigned int B_setup_shader(const char *vert_path, const char *frag_path);
int B_check_shader(unsigned int id, const char *name, int status);
void B_set_uniform_float(B_Shader shader, char *name, float value);
void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value);
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value);
void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value);
void B_set_uniform_int(B_Shader shader, char *name, int value);

#endif



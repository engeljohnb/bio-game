#ifndef __TESSELLATION_H__
#define __TESSELLATION_H__
#include <glad/glad.h>
#include "rendering.h"
#include "camera.h"

typedef struct
{
//	int		resolution;
	unsigned int	vao;
	unsigned int	vbo;
	int		num_vertices;
	int		num_columns;
//	unsigned int	texture;
} T_Mesh;

typedef struct
{
	GLfloat		position[3];
//	GLfloat		tex_coords[2];
} T_Vertex;

T_Vertex *generate_t_vertices(int width, int height);
T_Mesh B_send_terrain_mesh_to_gpu(T_Vertex *vertices, int num_vertices, int num_columns);
unsigned int B_compile_terrain_shader(const char *vert_path, const char *frag_path, const char *ctess_path, const char *etess_path);
void B_draw_terrain(T_Mesh mesh, mat4 world_space, B_Shader shader, Camera *camera);
T_Mesh B_create_terrain_mesh(int width, int height);
#endif
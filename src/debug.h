#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <cglm/cglm.h>
#include "common.h"

typedef struct Plane 
{
	B_Shader	shader;
	B_Framebuffer	g_buffer;
	unsigned int	vao;
	unsigned int	ebo;
	vec3 		color;
	int		indices[6];
} Plane;

typedef struct FrustumDebug
{
	B_Shader 	shader;
	B_Framebuffer 	g_buffer;
	unsigned int	vao;
	unsigned int	vbo;
	int		indices[6];
} FrustumDebug;

void B_draw_frustum_debug(FrustumDebug frustum_debug, mat4 projection_view);
FrustumDebug B_create_frustum_debug(vec4 planes[6], B_Shader shader, B_Framebuffer g_buffer);
Plane B_create_plane(B_Framebuffer g_buffer, vec3 color, B_Shader shader);
void draw_viewing_frustum(mat4 projection_view, Plane plane);
void set_alt_projection_view(mat4 src);
void get_alt_projection_view(mat4 dest);
void set_alt_projection(mat4 src);
void get_alt_projection(mat4 src);

#endif

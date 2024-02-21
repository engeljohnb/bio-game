#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "utils.h"
#include "debug.h"

mat4 g_alt_projection_view;
mat4 g_alt_projection;

FrustumDebug B_create_frustum_debug(vec4 planes[6], B_Shader shader, B_Framebuffer g_buffer)
{
	unsigned int vao = 0;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	FrustumDebug frustum_debug = {0};
	frustum_debug.vao = vao;
	int indices[] = { 0, 1, 2, 3, 4, 5 };
	for (int i = 0; i < 6; ++i)
	{
		frustum_debug.indices[i] = indices[i];
	}

	frustum_debug.shader = shader;
	frustum_debug.g_buffer = g_buffer;

	unsigned int vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	size_t stride = (6*(4*sizeof(float)));
	float data[(6*4)] = {0};
	for (int i = 0; i < 6; ++i)
	{
		data[0 + (i*4)] = planes[i][0];
		data[1 + (i*4)] = planes[i][1];
		data[2 + (i*4)] = planes[i][2];
		data[3 + (i*4)] = planes[i][3];
	}
	glBufferData(GL_ARRAY_BUFFER, stride, data, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	return frustum_debug;
}

void B_draw_frustum_debug(FrustumDebug frustum_debug, mat4 projection_view)
{
	glUseProgram(frustum_debug.shader);
	glBindVertexArray(frustum_debug.vao);
	B_set_uniform_mat4(frustum_debug.shader, "projection_view", projection_view);
	glDrawElements(GL_TRIANGLES, 6, GL_INT, frustum_debug.indices);
}

void set_alt_projection(mat4 src)
{
	glm_mat4_copy(src, g_alt_projection);
}

void get_alt_projection(mat4 dest)
{
	glm_mat4_copy(g_alt_projection, dest);
}

void set_alt_projection_view(mat4 src)
{
	glm_mat4_copy(src, g_alt_projection_view);
}

void get_alt_projection_view(mat4 dest)
{
	glm_mat4_copy(g_alt_projection_view, dest);
}

Plane B_create_plane(B_Framebuffer g_buffer, vec3 color, B_Shader shader)
{	
	unsigned int vao = 0;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	Plane plane = {0};
	plane.vao = vao;
	int indices[] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < 6; ++i)
	{
		plane.indices[i] = indices[i];
	}
	glm_vec3_copy(color, plane.color);

	plane.shader = shader;
	plane.g_buffer = g_buffer;
	return plane;
}

void B_draw_plane(Plane plane, vec3 corners[4], mat4 projection_view)
{
	unsigned int vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float), corners, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, plane.g_buffer);
	glUseProgram(plane.shader);
	B_set_uniform_vec3(plane.shader, "color", plane.color);
	B_set_uniform_mat4(plane.shader, "projection_view", projection_view);
	glBindVertexArray(plane.vao);
	glDrawElements(GL_TRIANGLES, 4, GL_INT, plane.indices);
	glFinish();
	glDeleteBuffers(1, &vbo);
}

void draw_viewing_frustum(mat4 projection_view, Plane plane)
{
	glDisable(GL_CULL_FACE);
	vec3 corners[8];
	get_frustum_corners(projection_view, corners);

	vec3 near_corners[4];
	glm_vec3_copy(corners[GLM_LBN], near_corners[0]);
	glm_vec3_copy(corners[GLM_LTN], near_corners[1]);
	glm_vec3_copy(corners[GLM_RTN], near_corners[2]);
	glm_vec3_copy(corners[GLM_RBN], near_corners[3]);
	B_draw_plane(plane, near_corners, projection_view);

	vec3 far_corners[4];
	glm_vec3_copy(corners[GLM_LBF], far_corners[0]);
	glm_vec3_copy(corners[GLM_LTF], far_corners[1]);
	glm_vec3_copy(corners[GLM_RTF], far_corners[2]);
	glm_vec3_copy(corners[GLM_RBF], far_corners[3]);
	B_draw_plane(plane, far_corners, projection_view);

	vec3 left_corners[4];
	glm_vec3_copy(corners[GLM_LBF], left_corners[0]);
	glm_vec3_copy(corners[GLM_LTF], left_corners[1]);
	glm_vec3_copy(corners[GLM_LTN], left_corners[2]);
	glm_vec3_copy(corners[GLM_LBN], left_corners[3]);
	B_draw_plane(plane, left_corners, projection_view);

	vec3 right_corners[4];
	glm_vec3_copy(corners[GLM_RBF], right_corners[0]);
	glm_vec3_copy(corners[GLM_RTF], right_corners[1]);
	glm_vec3_copy(corners[GLM_RTN], right_corners[2]);
	glm_vec3_copy(corners[GLM_RBN], right_corners[3]);
	B_draw_plane(plane, right_corners, projection_view);

	vec3 top_corners[4];
	glm_vec3_copy(corners[GLM_RTF], top_corners[0]);
	glm_vec3_copy(corners[GLM_RTN], top_corners[1]);
	glm_vec3_copy(corners[GLM_LTF], top_corners[2]);
	glm_vec3_copy(corners[GLM_LTN], top_corners[3]);
	B_draw_plane(plane, top_corners, projection_view);

	vec3 bottom_corners[4];
	glm_vec3_copy(corners[GLM_RBF], bottom_corners[0]);
	glm_vec3_copy(corners[GLM_RBN], bottom_corners[1]);
	glm_vec3_copy(corners[GLM_LBF], bottom_corners[2]);
	glm_vec3_copy(corners[GLM_LBN], bottom_corners[3]);
	B_draw_plane(plane, bottom_corners, projection_view);
	glEnable(GL_CULL_FACE);



}

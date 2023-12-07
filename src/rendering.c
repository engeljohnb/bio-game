/*
    Bio-Game is a game for designing your own organism. 
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
#include <stdlib.h>
#include "window.h"
#include "rendering.h"
#include "utils.h"
#include "time.h"

B_Framebuffer B_generate_g_buffer(B_Texture *normal_texture, B_Texture *position_texture, B_Texture *color_texture, unsigned int *lighting_vao, unsigned int *lighting_vbo)
{

	GLfloat texture_vertices[] =
	{
	// position		// tex_coords
	 -1.0f, -1.0f, 0.0f ,   0.0f, 0.0f, //0
	  1.0f, -1.0f, 0.0f,    1.0f, 0.0f, //2
	 -1.0f, 1.0f, 0.0f,	0.0f, 1.0f, //1
	  1.0f, 1.0f, 0.0f,     1.0f, 1.0f,  //5
	  -1.0f, 1.0f, 0.0f,    0.0f, 1.0f, //4
	  1.0f, -1.0f, 0.0f,    1.0f, 0.0f, //3
					    };

	size_t stride = sizeof(GLfloat)*5;

	B_Texture _normal_texture = 0;
	B_Texture _position_texture = 0;
	B_Texture _color_texture = 0;
	unsigned int _lighting_vao = 0;
	unsigned int _lighting_vbo = 0;
	B_Framebuffer g_buffer = 0;

	glGenVertexArrays(1, &_lighting_vao);
	glBindVertexArray(_lighting_vao);
	glGenBuffers(1, &_lighting_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _lighting_vbo);
	glBufferData(GL_ARRAY_BUFFER, 6*stride, texture_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenFramebuffers(1, &g_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glGenTextures(1, &_normal_texture);
	glBindTexture(GL_TEXTURE_2D, _normal_texture);

	int width = 0;
	int height = 0;
	get_window_size(&width, &height);
	if (height > 1440)
	{
		width /= 4;
		height /= 4;
	}
	else
	{
		height /= 2;
		width /= 2;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _normal_texture, 0);

	glGenTextures(1, &_position_texture);
	glBindTexture(GL_TEXTURE_2D, _position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _position_texture, 0);

	glGenTextures(1, &_color_texture);
	glBindTexture(GL_TEXTURE_2D, _color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB4, width, height, 0, GL_RGB, GL_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _color_texture, 0);

	unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	unsigned int depth_buffer;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

	*position_texture = _position_texture;
	*normal_texture = _normal_texture;
	*color_texture = _color_texture;
	*lighting_vao = _lighting_vao;
	*lighting_vbo = _lighting_vbo;
	return g_buffer;
}

Renderer create_default_renderer(B_Window window)
{
	Camera camera = create_camera(window, VEC3(0.0, 0.0, 0.0), VEC3_Z_DOWN);
	Renderer renderer;
	renderer.camera = camera;
	renderer.window = window;
	renderer.g_buffer = B_generate_g_buffer(&renderer.normal_texture, &renderer.position_texture, &renderer.color_texture,
						&renderer.lighting_vao, &renderer.lighting_vbo);
	return renderer;
}

void free_renderer(Renderer renderer)
{
	glDeleteBuffers(1, &renderer.lighting_vbo);
	glDeleteTextures(1, &renderer.normal_texture);
	glDeleteTextures(1, &renderer.position_texture);
	glDeleteVertexArrays(1, &renderer.lighting_vao);
	glDeleteFramebuffers(1, &renderer.g_buffer);
}

void B_render_lighting(Renderer renderer, 
		       B_Shader shader, 
		       PointLight player_light, 
		       DirectionLight weather_light,
		       DirectionLight tod_light,
		       vec3 sky_color,
		       vec3 camera_position,
		       vec3 player_position,
		       float rain_fog_percent,
		       float dew_fog_percent,
		       int mode)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer.position_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer.normal_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderer.color_texture);

	vec3 adjusted_camera_position;
	glm_vec3_scale(camera_position, 0.01, adjusted_camera_position);

	B_set_uniform_float(shader,"rain_fog_percent", rain_fog_percent);
	B_set_uniform_float(shader,"dew_fog_percent", dew_fog_percent);
	B_set_uniform_float(shader, "sea_level", SEA_LEVEL);
	B_set_uniform_int(shader, "f_position_texture", 1);
	B_set_uniform_int(shader, "f_normal_texture", 0);
	B_set_uniform_int(shader, "f_color_texture", 2);
	B_set_uniform_point_light(shader, "player_light", player_light);
	B_set_uniform_direction_light(shader, "weather_light", weather_light);
	B_set_uniform_direction_light(shader, "tod_light", tod_light);
	B_set_uniform_vec3(shader, "sky_color", sky_color);
	B_set_uniform_vec3(shader, "camera_position", adjusted_camera_position);
	B_set_uniform_vec3(shader, "player_position", player_position);
	B_set_uniform_int(shader, "mode", mode);
	B_set_uniform_float(shader, "view_distance", (get_view_distance()/100.0f));

	GLuint indices[] = { 0, 1, 2, 3, 4, 5 };
	glBindVertexArray(renderer.lighting_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);

	glDisable(GL_CULL_FACE);
}


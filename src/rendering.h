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

#ifndef __RENDER_H__
#define __RENDER_H__

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "camera.h"
#include "window.h"
#include "common.h"

/* Most objects in the game have rendering needs specific to that type of object. However, there are
 * a few things that will tend to be the same with most every object. A Renderer is just a
 * collection of any data needed for rendering which most objects should be able to share
 * (such as the deferred lighting framebuffer, for example).
 *
 * It's not really organized. Whenever I find myself passing the same variable to every draw function,
 * I just go ahead and throw it in the Renderer. */
typedef struct
{
	Camera		camera;
	B_Window	window;
	float		delta_t;
	B_Texture	normal_texture;
	B_Texture	position_texture;
	B_Texture	color_texture;
	B_Framebuffer	g_buffer;
	unsigned int	lighting_vao;
	unsigned int	lighting_vbo;
} Renderer;


B_Framebuffer B_generate_g_buffer(B_Texture *normal_texture, B_Texture *position_texture, B_Texture *color_texture, unsigned int *lighting_vao, unsigned int *lighting_vbo);
void B_render_lighting(Renderer renderer, 
		       B_Shader shader, 
		       PointLight point_light, 
		       DirectionLight weather_light,
		       vec3 sky_color,
		       vec3 camera_position,
		       int mode);
Renderer create_default_renderer(B_Window window);
void free_renderer(Renderer renderer);
#endif



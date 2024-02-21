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




#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <cglm/cglm.h>
#include "terrain_collisions.h"
#include "actor_state.h"
#include "window.h"
#include "input.h"

typedef struct Camera
{
	vec3		position;
	vec3 		move_direction;
	vec3		front;
	vec3		right;
	mat4		view_space;
	mat4		projection_space;
} Camera;


Camera create_camera(B_Window window, vec3 position, vec3 front);
void update_camera(Camera *camera, ActorState player, TerrainChunk *terrain_chunk, mat4 yaw_dest);
void look_at(Camera *camera, vec3 target);
void set_camera(Camera *camera, vec3 position, vec3 direction);
float get_view_distance(void);
float get_camera_height(void);

#endif

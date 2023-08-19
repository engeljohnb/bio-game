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
=
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "actor_state.h"
#include "window.h"
#include "input.h"
#include "utils.h"
#include "camera.h"
#include "time.h"

extern float delta_t;
float g_camera_height;

Camera create_camera(B_Window window, vec3 position, vec3 front)
{
	float view_distance = get_view_distance();
	Camera camera;
	memset(&camera, 0, sizeof(Camera));
	glm_perspective(RAD(45), (float)window.width/(float)window.height, 1.0f, view_distance, camera.projection_space);
	glm_vec3_copy(position, camera.position);
	glm_vec3_copy(front, camera.front);
	glm_vec3_copy(VEC3_X_UP, camera.right);

	glm_vec3_copy(VEC3_ZERO, camera.move_direction);

	mat4 rotation_dest;
	get_rotation_matrix(0, 0, rotation_dest);
	glm_mat4_mulv3(rotation_dest, VEC3_Z_DOWN, 0, camera.front);
	glm_mat4_mulv3(rotation_dest, VEC3_X_DOWN, 0, camera.right);

	vec3 frontpos;
	glm_vec3_add(front, position, frontpos);
	glm_lookat(frontpos, position, VEC3_Y_UP, camera.view_space);

	g_camera_height = camera.position[1];
	
	return camera;
}

void my_lookat(vec3 camera_center, vec3 target_center, vec3 up, mat4 target)
{
	vec3 forward;
	glm_vec3_sub(camera_center, target_center, forward);
	glm_vec3_normalize(forward);

	vec3 right;
	glm_vec3_cross(up, forward, right);
	glm_vec3_normalize(right);	

	mat4 result = { { right[0],         right[1],         right[2],         0},
		        { up[0],            up[1],            up[2],	        0},
			{ forward[0],       forward[1],       forward[2],       0 },
			{ camera_center[0], camera_center[1], camera_center[2], 1 } };

	glm_mat4_copy(result, target);

}

void update_camera(Camera *camera, ActorState player, TerrainChunk *terrain_chunk, mat4 rotation)
{
	static int camera_scroll = 55;
	camera_scroll += player.command_state.wheel_increment;
	glm_vec3_copy(player.position, camera->position);
	mat4 translate;
	glm_mat4_identity(translate);
	vec3 up;
	glm_vec3_cross(camera->front, camera->right, up);

	get_rotation_matrix(player.command_state.look_x, player.command_state.look_y, rotation);
	glm_mat4_mulv3(rotation, VEC3_Z_DOWN, 0, camera->front);
	glm_mat4_mulv3(rotation, VEC3_X_DOWN, 0, camera->right);

	vec3 camera_direction;
	glm_vec3_copy(camera->front, camera_direction);
	glm_vec3_scale(camera_direction, camera_scroll, camera_direction);
	glm_translate(translate, camera_direction);
	glm_mat4_mulv3(translate, player.position, 1, camera->position);

	float height = get_terrain_height(camera->position, terrain_chunk);
	if (camera->position[1] < (height + 3.0f))
	{
		camera->position[1] = height + 3.0f;
	}

	//camera->position[1] = 100.0f;

	vec3 target;
	glm_vec3_add(player.position, VEC3(0, 10.0f + ((float)camera_scroll-55.0f)/3.0f, 0), target);
	glm_lookat(camera->position, target, up, camera->view_space);

	g_camera_height = camera->position[1];
}

float get_camera_height(void)
{
	return g_camera_height;
}

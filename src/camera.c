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

Camera create_camera(B_Window window, vec3 position, vec3 front)
{
	Camera camera;
	memset(&camera, 0, sizeof(Camera));
	glm_perspective(RAD(45.0f), (float)window.width/(float)window.height, 0.1f, 1000.0f, camera.projection_space);
	glm_vec3_copy(position, camera.position);
	glm_vec3_copy(front, camera.front);
	glm_vec3_copy(VEC3_X_UP, camera.right);

	vec3 frontpos;
	glm_vec3_add(front, position, frontpos);

	glm_vec3_copy(VEC3_ZERO, camera.move_direction);

	glm_lookat(frontpos, position, VEC3_Y_UP, camera.view_space);
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

void update_camera(Camera *camera, ActorState player, mat4 euler_dest)
{
	glm_vec3_copy(player.position, camera->position);
        turn(camera->front, player.command_state.look_x, player.command_state.look_y, VEC3_Z_DOWN, euler_dest);
        turn(camera->right, player.command_state.look_x, player.command_state.look_y, VEC3_X_DOWN, NULL);
	mat4 translate;
	glm_mat4_identity(translate);
	vec3 up;
	glm_vec3_cross(camera->front, camera->right, up);

	vec3 camera_direction;
	glm_vec3_copy(camera->front, camera_direction);
	//glm_vec3_negate(camera_direction);
	glm_vec3_scale(camera_direction, 35, camera_direction);
	glm_translate(translate, camera_direction);
	glm_mat4_mulv3(translate, player.position, 1, camera->position);

	glm_lookat(camera->position, player.position, up, camera->view_space);
}

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

Camera create_camera(B_Window window, vec3 position, vec3 front, vec3 up)
{
	Camera camera;
	memset(&camera, 0, sizeof(Camera));
	camera.speed = 0.0;
	camera.max_speed = 0.03;
	glm_perspective(RAD(45.0f), (float)window.width/(float)window.height, 0.1f, 100.0f, camera.projection_space);
	glm_vec3_copy(position, camera.position);
	glm_vec3_copy(front, camera.front);
	glm_vec3_copy(up, camera.up);
	glm_vec3_zero(camera.right);

	vec3 right;
	glm_vec3_cross(front, up, right);
	glm_vec3_normalize(right);
	glm_vec3_copy(right, camera.right);

	vec3 frontpos;
	glm_vec3_add(front, position, frontpos);

	glm_vec3_copy(VEC3_ZERO, camera.move_direction);

	glm_lookat(frontpos, position, up, camera.view_space);
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

void update_camera(Camera *camera, ActorState player)
{
	static float look_x = -90;
	look_x += 0.1;
	player.command_state.look_x = look_x;
	glm_vec3_copy(player.position, camera->position);
	vec3 prev_front;
	glm_vec3_copy(camera->front, prev_front);
	turn(camera->front, player.command_state.look_x, player.command_state.look_y);
	mat4 translate;
	glm_mat4_identity(translate);

	vec3 camera_direction;
	glm_vec3_copy(camera->front, camera_direction);
	glm_vec3_scale(camera_direction, 5, camera_direction);
	glm_translate(translate, camera_direction);
	glm_mat4_mulv3(translate, player.position, 1, camera->position);

	glm_vec3_copy(player.up, camera->up);

	glm_lookat(camera->position, player.position, camera->up, camera->view_space);
	float angle = glm_vec3_angle(VEC3_Z_DOWN, camera->front);//, -1.0, 1.0);
	vec3 axis;
	glm_vec3_cross(VEC3_Z_DOWN, camera->front, axis);
	glm_vec3_normalize(axis);
	glm_vec3_copy(axis, camera->rotation_axis);
	camera->rotation_angle = angle;
}

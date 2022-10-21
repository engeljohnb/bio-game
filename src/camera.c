#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "input.h"
#include "utils.h"
#include "camera.h"
#include "time.h"

extern float delta_t;
Camera create_camera(vec3 position, vec3 front, vec3 up)
{
	Camera camera;
	camera.speed = 0.0;
	camera.max_speed = 0.03;
	glm_perspective(RAD(45.0f), 800.0f/600.0f, 0.1f, 100.0f, camera.projection_space);
	glm_vec3_copy(position, camera.position);
	glm_vec3_copy(front, camera.front);
	glm_vec3_copy(up, camera.up);

	vec3 frontpos;
	glm_vec3_add(front, position, frontpos);
	glm_lookat(position, frontpos, up, camera.view_space);

	glm_vec3_copy(VEC3_ZERO, camera.move_direction);
	glm_vec3_copy(VEC3_ZERO, camera.yaw);
	glm_vec3_copy(VEC3_ZERO, camera.pitch);

	return camera;
}

void update_camera(Camera *camera, CommandState command_state)
{
	if (command_state.movement & M_FORWARD)
	{
		glm_vec3_copy(VEC3_Z_UP, camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_BACKWARD)
	{
		glm_vec3_copy(VEC3_Z_DOWN, camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_LEFT)
	{
		glm_vec3_copy(VEC3_X_UP, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_RIGHT)
	{
		glm_vec3_copy(VEC3_X_DOWN, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}

	if (camera->speed > camera->max_speed)
	{
		camera->speed = camera->max_speed;
	}

	if (camera->speed < 0)
	{
		camera->speed = 0;
	}
	// camera->position += camera->speed * camera->move_dir;
	vec3 speed_movedir;
	glm_vec3_scale(camera->move_direction, camera->speed, speed_movedir);
	glm_vec3_add(camera->position, speed_movedir, camera->position);

	vec3 frontpos;
	glm_vec3_add(camera->front, camera->position, frontpos);
	glm_lookat(camera->position, frontpos, camera->up, camera->view_space);

	if (command_state.movement == 0)
	{
		camera->speed -= 0.0001*delta_t;
		//camera->speed = 0;
	}

}

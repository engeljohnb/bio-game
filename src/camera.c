#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "window.h"
#include "input.h"
#include "utils.h"
#include "camera.h"
#include "time.h"

extern float delta_t;

Camera create_camera(B_Window window, vec3 position, vec3 front, vec3 up)
{
	Camera camera;
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
	glm_vec3_copy(VEC3_ZERO, camera.yaw);
	glm_vec3_copy(VEC3_ZERO, camera.pitch);

	glm_lookat(frontpos, position, up, camera.view_space);
	return camera;
}
/*
void swivel_camera(Camera *camera, float x, float y, vec3 target)
{

}*/

void rotate_camera(Camera *camera, float x, float y)
{
	 glm_vec3_copy(VEC3(cos(RAD(x)) * cos(RAD(y)), 
				 sin(RAD(y)), 
				 sin(RAD(x))*cos(RAD(y))), 
			camera->front);
	 glm_normalize(camera->front);
}

void update_camera(Camera *camera, CommandState command_state)
{
	if (command_state.movement & M_BACKWARD)
	{
		vec3 front;
		glm_vec3_copy(camera->front, front);
		glm_vec3_normalize(front);
		glm_vec3_add(camera->front, camera->move_direction, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_FORWARD)
	{
		vec3 backward = {0.0, 0.0, 0.0};
		glm_vec3_negate_to(camera->front, backward);
		glm_normalize(backward);
		glm_vec3_add(backward, camera->move_direction, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_RIGHT)
	{
		vec3 left = {0.0, 0.0, 0.0};
		glm_vec3_cross(camera->front, camera->up, left);
		glm_vec3_negate(left);
		glm_vec3_normalize(left);
		glm_vec3_add(left, camera->move_direction, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}
	if (command_state.movement & M_LEFT)
	{
		vec3 right = {0.0, 0.0, 0.0};
		glm_vec3_cross(camera->front, camera->up, right);
		glm_normalize(right);
		glm_vec3_add(right, camera->move_direction, camera->move_direction);
		glm_vec3_normalize(camera->move_direction);
		camera->speed += 0.0001*delta_t;
	}

	// I have no idea why, but the player moves the opposite direction I think they should. Temporary solution until I actually figure it out.
	//glm_vec3_negate(camera->move_direction);

	if (camera->speed > camera->max_speed)
	{
		camera->speed = camera->max_speed;
	}

	if (camera->speed < 0)
	{
		camera->speed = 0;
	}

	if (command_state.look_x_increment || command_state.look_y_increment)
	{
		camera->look_x += command_state.look_x_increment;
		camera->look_y += command_state.look_y_increment;
		rotate_camera(camera, camera->look_x, camera->look_y);
	}
	
	// camera->position += camera->speed * camera->move_dir;
	vec3 movement = { 0.0, 0.0, 0.0 };
	glm_vec3_scale(camera->move_direction, camera->speed, movement);
	glm_vec3_add(camera->position, movement, camera->position);

	if (command_state.movement == 0)
	{
		glm_vec3_copy(VEC3_ZERO, camera->move_direction);
		camera->speed -= 0.0001*delta_t;
	}

	vec3 frontpos = {0.0, 0.0, 0.0};
	glm_vec3_add(camera->position, camera->front, frontpos);
	glm_lookat(frontpos, camera->position, camera->up, camera->view_space);
}

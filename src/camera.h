#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <cglm/cglm.h>
#include "input.h"

typedef struct
{
	float	speed;
	float	max_speed;
	vec3	position;
	vec3 	move_direction;
	vec3	front;
	vec3	up;
	vec3	yaw;
	vec3	pitch;
	mat4	view_space;
	mat4	projection_space;
} Camera;


Camera create_camera(vec3 position, vec3 front, vec3 up);
void update_camera(Camera *camera, CommandState command_state);

#endif

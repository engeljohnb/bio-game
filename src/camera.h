#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <cglm/cglm.h>
#include "actor_state.h"
#include "window.h"
#include "input.h"

typedef struct
{
	vec3	position;
	vec3 	move_direction;
	vec3	front;
	mat4	view_space;
	mat4	projection_space;
} Camera;


Camera create_camera(B_Window window, vec3 position, vec3 front);
//void update_camera(Camera *camera, CommandState command_state, float delta_t);
void update_camera(Camera *camera, ActorState player, mat4 euler_dest);
void look_at(Camera *camera, vec3 target);

#endif

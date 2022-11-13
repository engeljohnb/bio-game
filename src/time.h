#ifndef __TIME_H__
#define __TIME_H__

#include <SDL2/SDL.h>
#define FPS_30 33

void B_keep_time(int target_period);
float B_get_frame_time(float delta_t);

#endif

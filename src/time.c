#include "time.h"

float B_get_frame_time(float delta_t)
{
	static unsigned int prev_time = 0;
	unsigned long current_time = SDL_GetTicks();
	float frame_time = (float)(current_time - prev_time);
	if (frame_time < delta_t)
	{
		float remainder = delta_t - frame_time;
		current_time += remainder;
		SDL_Delay((unsigned int)remainder);
		frame_time = delta_t;
	}
	prev_time = current_time;
	return frame_time;
}

void B_keep_time(int target_period)
{
	static unsigned int prev_time = 0;
	static float delta_t = 15.0;
	unsigned int current_time = SDL_GetTicks();
	delta_t = (float)(current_time - prev_time);
	if (delta_t < target_period)
	{
		SDL_Delay(target_period - delta_t);
		delta_t = (float)target_period;
		current_time += target_period - delta_t;
	}
	prev_time = current_time;

}

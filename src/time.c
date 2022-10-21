#include "time.h"

float delta_t = 33.0;

void B_keep_time(int target_period)
{
	static unsigned int prev_time = 0;
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

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

#include "time.h"
#include "glad/glad.h"

float B_get_frame_time(void)
{
	static unsigned int prev_time = 0;
	unsigned long current_time = SDL_GetTicks();
	float frame_time = (float)(current_time - prev_time);
	/*if (frame_time < delta_t)
	{
		float remainder = delta_t - frame_time;
		current_time += remainder;
		SDL_Delay((unsigned int)remainder);
		frame_time = delta_t;
	}*/
	prev_time = current_time;
	return frame_time;
}

void B_stopwatch(char *message)
{
	glFinish();
	static unsigned long prev_time = 0;
	unsigned long time = SDL_GetTicks();
	fprintf(stdout, "%lu %s\n", (time-prev_time), message);
	prev_time = time;
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

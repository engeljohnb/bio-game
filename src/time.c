/*
    Bio-Game is a game for designing your own organism. 
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

double B_get_current_second(void)
{
	uint64_t current_time = SDL_GetTicks64();
	return (double)floor((current_time % 60000)/1000.0);
}

double B_get_current_minute(void)
{
	uint64_t current_time = SDL_GetTicks64();
	return (double)floor((current_time / 60000) % 60);
}

double B_get_current_playtime_hour(void)
{
	uint64_t current_time = SDL_GetTicks64();
	return (double)floor((current_time / (60000*60*24))%24);
}

double B_get_current_in_game_hour(void)
{
	uint64_t current_time = SDL_GetTicks64();
	return (double)floor(fmodf(current_time / (6000*MINUTES_PER_IN_GAME_DAY), MINUTES_PER_IN_GAME_DAY));
}

double B_get_seconds_into_current_phase(void)
{
	return fmodf(B_get_current_second() + (B_get_current_minute()*60), SECONDS_PER_IN_GAME_DAY);
}

float B_get_frame_time(void)
{
	static unsigned int prev_time = 0;
	//TODO: Change this to SDL_GetTicks64
	unsigned long current_time = SDL_GetTicks64();
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
